#include "vm/frame.h"
#include "vm/swap.h"
#include "devices/disk.h"
#include "devices/timer.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "filesys/file.h"
#include <bitmap.h>
#include <list.h>
#include <hash.h>
#include <string.h>

#define DISK_SECTORS_PER_PAGE (PGSIZE / DISK_SECTOR_SIZE)
#define FREE 0
#define USED 1

/* The swap device */
static struct disk *swap_device;

/* Tracks in-use and free swap slots. */
static struct bitmap *swap_table;

/* Protects swap_table */
static struct lock swap_lock;

static void read_from_disk (uint8_t *frame, size_t index);
static void write_to_disk (uint8_t *frame, size_t index);

/* 
 * Initialize swap_device, swap_table, and swap_lock.
 */
void 
swap_init (void)
{
  swap_device = disk_get (1, 1);
  swap_table = bitmap_create (disk_size (swap_device) / DISK_SECTORS_PER_PAGE);
  if (swap_device == NULL || swap_table == NULL)
    PANIC ("Swap initialization fail: out of memory");
  bitmap_set_all (swap_table, FREE);
  lock_init (&swap_lock);
}

/*
 * Reclaim a frame from swap device.
 * 1. Check that the page has been already evicted. 
 * 2. You will want to evict an already existing frame
 * to make space to read from the disk to cache. 
 * 3. Re-link the new frame with the corresponding supplementary
 * page table entry. 
 * 4. Do NOT create a new supplementray page table entry. Use the 
 * already existing one. 
 * 5. Use helper function read_from_disk in order to read the contents
 * of the disk into the frame. 
 */ 
bool 
swap_in (void *addr, bool on)
{
  void *kpage;
  size_t sector;
  struct page_table_entry *spte;
  struct thread *t;
  
  spte = find_spte (addr);
  t = thread_current ();

  /* Get a new page. */
  kpage = palloc_get_page (PAL_USER);
  while (kpage == NULL)
    {
      kpage = swap_out ();
    }

  /* Get suplement page table entry. */
  if (spte == NULL || spte->exist)
    {
      ASSERT (0);
    }

  /* Restore supplement table entry & frame table. 
     Also, free corrsponding swap slot. */
  lock_acquire (&spte->in_process_lock);
  if (pagedir_set_page (t->pagedir, spte->uvaddr, kpage, spte->writable) == false)
    {
      lock_release (&spte->in_process_lock);
      palloc_free_page (kpage);
      return false;
    }

  if (spte->exist_in_swap_disk)
    {
      sector = spte->sector;
      read_from_disk ((uint8_t *) kpage, sector);

      lock_acquire (&swap_lock);
      bitmap_flip (swap_table, sector);
      lock_release (&swap_lock);
    }
  else if (spte->file != NULL && spte->read_size > 0)
    {
       lock_acquire (&filesys_lock);
       file_read_at (spte->file, kpage, spte->read_size, spte->of);
       lock_release (&filesys_lock);

       memset ((uint8_t *)kpage + spte->read_size, 0, PGSIZE - spte->read_size);
    }
  else
    {
      memset (kpage, 0, PGSIZE);
    }
 
  /* Set supplement page table entry. */
  spte->frame = kpage;
  spte->exist_in_swap_disk = false;
  spte->exist = true;

  /* Set frame table entry */
  allocate_frame (kpage, spte, on);

  lock_release (&spte->in_process_lock);
  return true;
}

/* 
 * Evict a frame to swap device. 
 * 1. Choose the frame you want to evict. 
 * (Ex. Least Recently Used policy -> Compare the timestamps when each 
 * frame is last accessed)
 * 2. Evict the frame. Unlink the frame from the supplementray page table entry
 * Remove the frame from the frame table after freeing the frame with
 * pagedir_clear_page. 
 * 3. Do NOT delete the supplementary page table entry. The process
 * should have the illusion that they still have the page allocated to
 * them. 
 * 4. Find a free block to write you data. Use swap table to get track
 * of in-use and free swap slots.
 */
void *
swap_out (void)
{
  size_t sector;
  void *frame;
  struct page_table_entry *spte;
  struct thread *t = thread_current ();

  /* Find a frame to evict */
  spte = frame_find_evict ();

  /* Nothing to evict. */
  if (spte == NULL)
    {
      return NULL;
    }

  frame = spte->frame;

  /* Enroll swap_list and send date to swap slot if dirty. */
  if (spte->modified || pagedir_is_dirty (t->pagedir, spte->uvaddr))
    {
      if (spte->type == EXEC)
        {
          /* Get a swap slot */
          lock_acquire (&swap_lock);
          sector = bitmap_scan_and_flip (swap_table, 0, 1, FREE);
          lock_release (&swap_lock);
      
          if (sector == BITMAP_ERROR)
            PANIC ("Swap out fail: out of memeory");
      
          /* Send data. */
          write_to_disk ((uint8_t *) frame, sector);

          /* Set infomation to supplement page table entry. */
          spte->exist_in_swap_disk = true;
          spte->sector = sector;
          spte->modified = true;
        }
      else if (spte->type == MMAP)
        {
          lock_acquire (&filesys_lock);
          file_write_at (spte->file, frame, spte->read_size, spte->of);
          lock_release (&filesys_lock);
        }
      else
        ASSERT (0); /* Cannot reach. */
    }
  
  lock_release (&spte->in_process_lock);
  
  return frame;
}

/*
 * Delete swap slots that contains this thread's data.
 */
void
swap_slot_delete (struct page_table_entry *spte)
{
  if (!spte->exist && spte->exist_in_swap_disk)
    {
      lock_acquire (&swap_lock);
      bitmap_flip (swap_table, spte->sector);
      lock_release (&swap_lock);
    }
}

/* 
 * Read data from swap device to frame. 
 * Look at device/disk.c
 */
static void 
read_from_disk (uint8_t *frame, size_t index)
{
  int i;
  for (i = 0; i < DISK_SECTORS_PER_PAGE; i++)
    {
      disk_read (swap_device, index * DISK_SECTORS_PER_PAGE + i, frame);
      frame += DISK_SECTOR_SIZE;
    }
}

/* Write data to swap device from frame */
static void 
write_to_disk (uint8_t *frame, size_t index)
{
  int i;
  for (i = 0; i < DISK_SECTORS_PER_PAGE; i++)
    {
      disk_write (swap_device, index * DISK_SECTORS_PER_PAGE + i, frame);
      frame += DISK_SECTOR_SIZE;
    }
}


