#include "devices/timer.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/interrupt.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "filesys/file.h"

static struct frame_table *ft;

/*
 * Hash function.
 */
static uint32_t
fte_hash (const struct hash_elem *h, void *aux UNUSED)
{
  const struct frame_table_entry *fte = hash_entry (h, struct frame_table_entry, fte_elem);
  return hash_bytes (&fte->frame, sizeof (fte->frame));
}

/*
 * Comparison function.
 */
static bool
fte_less (const struct hash_elem *h1, const struct hash_elem *h2, void *aux UNUSED)
{
  const struct frame_table_entry *fte1 = hash_entry (h1, struct frame_table_entry, fte_elem);
  const struct frame_table_entry *fte2 = hash_entry (h2, struct frame_table_entry, fte_elem);
  return fte1->frame < fte2->frame;
}

/*
 * Initialize frame table
 */
void 
frame_init (void)
{
  ft = (struct frame_table *) malloc (sizeof (struct frame_table));

  /* Not enough kernel memory. */
  if (ft == NULL)
    PANIC ("Frame initialization fail: out of memory");

  list_init (&ft->delete_list);
  hash_init (&ft->fte_hash, fte_hash, fte_less, &ft->delete_list);
  lock_init (&ft->ft_lock);
}


/* 
 * Make a new frame table entry for addr.
 */
bool
allocate_frame (void *k_va, void *spte, bool on)
{
  struct frame_table_entry *fte = 
	(struct frame_table_entry *) malloc (sizeof (struct frame_table_entry));
  
  /* Out of kernel memory. */  
  if (fte == NULL)
    return false;
    
  /* Initialize frame table entry. */
  fte->frame = (uint8_t *) pg_round_down (k_va);
  fte->count = timer_ticks ();
  fte->fixed = on;
  fte->owner = thread_current ();
  fte->spte = spte;

  /* Insert it in frame table. */
  lock_acquire (&ft->ft_lock);
  hash_insert (&ft->fte_hash, &fte->fte_elem);
  lock_release (&ft->ft_lock);

  return true;
}

void 
unfix (void *addr)
{
  struct frame_table_entry fte_sample, *fte;
  struct page_table_entry *spte;
  struct hash_elem *h;

  spte = find_spte (addr);
  fte_sample.frame = spte->frame;

  lock_acquire (&ft->ft_lock);
  h = hash_find (&ft->fte_hash, &fte_sample.fte_elem);
  if (h == NULL)
    {
      lock_release (&ft->ft_lock);
      return;
    }
  
  fte = hash_entry (h, struct frame_table_entry, fte_elem);
  fte->fixed = false;
  lock_release (&ft->ft_lock);
}

void
fix (void *addr)
{
  struct frame_table_entry fte_sample, *fte;
  struct page_table_entry *spte;
  struct hash_elem *h;

  spte = find_spte (addr);
  fte_sample.frame = spte->frame;

  lock_acquire (&ft->ft_lock);
  h = hash_find (&ft->fte_hash, &fte_sample.fte_elem);
  if (h == NULL)
    {
      lock_release (&ft->ft_lock);
      return;
    }
  
  fte = hash_entry (h, struct frame_table_entry, fte_elem);
  fte->fixed = true;
  lock_release (&ft->ft_lock);

}

/* 
 * Delete a frame.
 * It is used in destroy_pages ().
 */
void
delete_frame (struct page_table_entry *spte)
{
  struct thread *t = thread_current ();
  struct hash_elem *h;
  struct frame_table_entry *fte, fte_sample;
  
  fte_sample.frame = spte->frame;

  lock_acquire (&ft->ft_lock);
  h = hash_delete (&ft->fte_hash, &fte_sample.fte_elem);
  if (h == NULL)
    {
      lock_release (&ft->ft_lock);
      return;
    }
  
  fte = hash_entry (h, struct frame_table_entry, fte_elem);
  free (fte);
  
  lock_release (&ft->ft_lock);

  if ((spte->modified || pagedir_is_dirty (t->pagedir, spte->uvaddr)) && spte->type == MMAP)
    {
      file_write_at (spte->file, spte->frame, spte->read_size, spte->of);
    }
}
    
/*
 * Check whether a frame is accessed/modified or not.
 * If accessed/modified, increase COUNT or set MODIFIED bit to TRUE.
 * This function is for frame_update_status ().
 */
static void
frame_check_status (struct hash_elem *h, void *aux UNUSED)
{
  struct frame_table_entry *fte = hash_entry (h, struct frame_table_entry, fte_elem);
 
  if (pagedir_is_accessed (fte->owner->pagedir, fte->spte->uvaddr))
    {
      fte->count++;
      pagedir_set_accessed (fte->owner->pagedir, fte->spte->uvaddr, false);
    }

  if (pagedir_is_dirty (fte->owner->pagedir, fte->spte->uvaddr))
    {
      fte->count++;
      pagedir_set_dirty (fte->owner->pagedir, fte->spte->uvaddr, false);

      fte->spte->modified = true;
    }
}

/* 
 * Count # of access for all frames.
 * Only used in devices/timer.c
 */
void
frame_update_status ()
{
  /* Count whether accessed or not. */
  hash_apply (&ft->fte_hash, frame_check_status);
}

/*
 * Find a least accessed frame.
 * This function is for frame_find_evict (). 
 */
static void
frame_min_access (struct hash_elem *h, void *aux)
{
  struct list *delete_list = (struct list *) aux;
  struct frame_table_entry *fte = hash_entry (h, struct frame_table_entry, fte_elem);

  /* Cannot evict in used page. */
  if (fte->fixed == true)
    return;
  
  if (list_empty (delete_list))
    {
      /* Insert first element. */
      struct delete_list_entry *dle = (struct delete_list_entry *) malloc (sizeof (struct delete_list_entry));
      dle->h = h;
      list_push_front (delete_list, &dle->dle_elem);
    }
  else
    {
      /* Compare access count. */
      struct list_elem *e = list_begin (delete_list);
      struct delete_list_entry *dle = list_entry (e, struct delete_list_entry, dle_elem);
      struct frame_table_entry *fte_cur = hash_entry (h, struct frame_table_entry, fte_elem);
      struct frame_table_entry *fte_min = hash_entry (dle->h, struct frame_table_entry, fte_elem);
      
      if (fte_cur->count < fte_min->count)
        dle->h = h;
    }
}

/*
 * Find the least accesssed frame and remove from fte_list.
 */
struct page_table_entry *
frame_find_evict ()
{
  struct frame_table_entry *fte;
  struct list_elem *e;
  struct hash_elem *h;
  struct delete_list_entry *dle;
  struct page_table_entry *spte;

  lock_acquire (&ft->ft_lock);

  hash_apply (&ft->fte_hash, frame_min_access);
 
  if (list_empty (&ft->delete_list))
    {
      intr_enable ();
      lock_release (&ft->ft_lock);
      return NULL; 
    }

  e = list_pop_front (&ft->delete_list);
  dle = list_entry (e, struct delete_list_entry, dle_elem);
  h = dle->h;

  free (dle);
  fte = hash_entry (h, struct frame_table_entry, fte_elem);
  spte = fte->spte;

  lock_acquire (&spte->in_process_lock);
  hash_delete (&ft->fte_hash, h);

  pagedir_clear_page (fte->owner->pagedir, fte->spte->uvaddr);

  spte->exist = false;
  
  free (fte);

  lock_release (&ft->ft_lock);

  return spte;
}

void *
stack_grow (void *addr, bool on)
{
  /* Handle stack growth. */
  struct thread *curr = thread_current ();
  void *kpage = palloc_get_page (PAL_USER);
  void *upage = pg_round_down (addr);
  struct page_table_entry *spte; 
  struct frame_table_entry *fte;

  while (kpage == NULL)
    kpage = swap_out ();
 
  spte = allocate_page (upage, kpage, true, NULL, 0, 0);
  fte = (struct frame_table_entry *) malloc (sizeof (struct frame_table_entry));
  
  /* Out of kernel memory. */  
  if (fte == NULL || spte == NULL || !pagedir_set_page (curr->pagedir, upage, kpage, true))
    {
      if (!fte) 
        free (fte);
      if (!spte)
        free (spte);

      PANIC ("Stack growth fail: out of memory");
    }
    
  /* Initialize frame table entry. */
  fte->frame = (uint8_t *) pg_round_down (kpage);
  fte->count = timer_ticks ();
  fte->fixed = on;
  fte->owner = curr;
  fte->spte = spte;

  /* Insert it in frame table. */
  lock_acquire (&ft->ft_lock);
  hash_insert (&ft->fte_hash, &fte->fte_elem);
  lock_release (&ft->ft_lock);
  
  /* Increase stack page count. */
  curr->process_stack_page_count++;    

  return upage;
}

