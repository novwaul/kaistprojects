#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "vm/page.h"
#include "filesys/file.h"

static uint32_t spte_hash (const struct hash_elem *h, void *aux);
static bool spte_less (const struct hash_elem *h1, const struct hash_elem *h2, void *aux);

/*
 * Initialize supplementary page table.
 */
bool 
sup_page_table_init (void)
{
  struct thread *curr = thread_current ();
  curr->spt = (struct hash *) malloc (sizeof (struct hash));
  if (curr->spt == NULL)
    PANIC ("Supplment page table initialization fail: out of memory");

  hash_init (curr->spt, spte_hash, spte_less, NULL);
  return true;
}

/*
 * Make new supplementary page table entry for addr.
 */
struct page_table_entry *
allocate_page (void *addr, void *frame, bool writable, struct file* file, off_t rs, off_t of)
{
  struct page_table_entry *spte = 
         (struct page_table_entry *) malloc (sizeof (struct page_table_entry));
  struct thread *curr = thread_current ();
  struct hash_elem *h;

  if (spte == NULL)
    return NULL;

  spte->uvaddr = (uint8_t *) pg_round_down (addr);
  spte->frame = (uint8_t *) pg_round_down (frame);
  spte->writable = writable;
  spte->exist = true;
  spte->exist_in_swap_disk = false;
  spte->file = file;
  spte->read_size = rs;
  spte->type = EXEC;
  spte->of = of;
  spte->modified = false;
  lock_init (&spte->in_process_lock);

  h = hash_insert (curr->spt, &spte->spte_elem);
  
  ASSERT (h == NULL);

  return spte;
}

/*
 * Allocate mmap. 
 */
struct page_table_entry *
allocate_mmap (void *addr, void *frame, struct file *file, off_t rs, off_t of)
{
  struct page_table_entry *spte = allocate_page (addr, frame, true, file, rs, of);
  spte->type = MMAP;

  return spte;
}

/*
 * Deallocate supplementary page table entry for addr.
 */
struct page_table_entry *
deallocate_page (void *addr)
{
  struct page_table_entry spte;
  struct thread *curr = thread_current ();
  struct hash_elem *h;
  
  spte.uvaddr = (uint8_t *) pg_round_down (addr);
  h = hash_delete (curr->spt, &spte.spte_elem);

  ASSERT (h != NULL);

  return hash_entry (h, struct page_table_entry, spte_elem);
}

/*
 * Hash function for supplementary page table.
 */
static uint32_t
spte_hash (const struct hash_elem *h, void *aux UNUSED)
{
  const struct page_table_entry *spte = hash_entry (h, struct page_table_entry, spte_elem);
  return hash_bytes (&spte->uvaddr, sizeof (spte->uvaddr));
}

/*
 * Comparison function for supplementary page table.
 */
static bool
spte_less (const struct hash_elem *h1, const struct hash_elem *h2, void *aux UNUSED)
{
  const struct page_table_entry *spte1 = hash_entry (h1, struct page_table_entry, spte_elem);
  const struct page_table_entry *spte2 = hash_entry (h2, struct page_table_entry, spte_elem);
  return spte1->uvaddr < spte2->uvaddr;
}

bool
is_writable (void *addr)
{
  struct page_table_entry spte_sample, *spte;
  struct hash_elem *h;
  struct thread *curr = thread_current ();
  spte_sample.uvaddr = (uint8_t *) pg_round_down (addr);
  h = hash_find (curr->spt, &spte_sample.spte_elem);
  spte = hash_entry (h, struct page_table_entry, spte_elem);
  return spte->writable;
}

bool
is_exist (void *addr)
{
  struct page_table_entry spte_sample, *spte;
  struct hash_elem *h;
  struct thread *curr = thread_current ();
  spte_sample.uvaddr = (uint8_t *) pg_round_down (addr);
  h = hash_find (curr->spt, &spte_sample.spte_elem);
  spte = hash_entry (h, struct page_table_entry, spte_elem);
  return spte->exist;
}

struct page_table_entry *
find_spte (void *addr)
{
  struct page_table_entry spte_sample, *spte;
  struct hash_elem *h;
  struct thread *t = thread_current ();
  
  spte_sample.uvaddr = (uint8_t *) pg_round_down (addr);
  h = hash_find (t->spt, &spte_sample.spte_elem);
  if (h == NULL)
    return NULL;

  spte = hash_entry (h, struct page_table_entry, spte_elem);
  return spte;
}

/*
 * Delete supplement page, frame, and swap slot. 
 */
static void
delete_page (struct hash_elem *h, void *aux UNUSED)
{
  struct page_table_entry *spte = hash_entry (h, struct page_table_entry, spte_elem);
  delete_frame (spte); 
  swap_slot_delete (spte);
  lock_acquire (&spte->in_process_lock);  /* No one will require the lock. */
  if (spte->type == MMAP)
    {
      file_close (spte->file);
    }
  free (spte);
}

/*
 * Destroy all pages.
 */
void destroy_pages (void)
{
  struct thread *curr = thread_current ();
  hash_destroy (curr->spt, delete_page);
  free (curr->spt);
}
