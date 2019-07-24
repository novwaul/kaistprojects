#include "devices/timer.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "filesys/cache.h"
#include "filesys/filesys.h"
#include <list.h>
#include <string.h>

#define WRITE_FREQ 1000
#define MAX_CACHE_CNT 64

struct lock cache_lock;		/* Lock for cache_hash. */
struct hash cache_hash;		/* Hash for cache. */
struct hash removed_hash;	/* Hash to contain cache entries that will be stored in disk. */
uint32_t cache_cnt;		/* # of cache_entries. */
struct cache_entry *ce_out;	/* An cache entry to be evicted. */
struct list write_back_list;	/* Cache entries to be written back to disk. 
                                   Only accessed by WRITE_BEHIND thread. */

struct write_back_entry
{
  size_t sector;
  struct list_elem write_back_elem;
};

static uint32_t cache_hashing (const struct hash_elem *h, void *aux UNUSED);
static bool cache_less (const struct hash_elem *h1, const struct hash_elem *h2, void *aux UNUSED);
static void check_write_behind (struct hash_elem *h, void *aux UNUSED);
static void do_cache_evict (struct hash_elem *h, void *aux UNUSED);
static void write_behind (void *aux UNUSED);
static void read_ahead (void *aux);
static void do_flush_cache (struct hash_elem *h, void *aux UNUSED);

/* Cache system initialization. */
void
cache_init ()
{
  lock_init (&cache_lock);
  hash_init (&cache_hash, cache_hashing, cache_less, NULL);
  hash_init (&removed_hash, cache_hashing, cache_less, NULL);
  list_init (&write_back_list);
  cache_cnt = 0;
  ce_out = NULL;
  thread_create ("write_behind", PRI_DEFAULT, write_behind, NULL);
}

/* Write behind function. */
static void
write_behind (void *aux UNUSED)
{
  struct list_elem *e;
  struct hash_elem *h;
  struct cache_entry *ce, ce_sample;
  struct write_back_entry *wbe;
  size_t sector;

  while (true)
   {
     timer_sleep (WRITE_FREQ);

     /* Get write back candidates. */
     lock_acquire (&cache_lock);
     hash_apply (&cache_hash, check_write_behind);
     lock_release (&cache_lock);
 
     while (!list_empty (&write_back_list))
       {
         e = list_pop_front (&write_back_list);
         wbe = list_entry (e, struct write_back_entry, write_back_elem);
         
	 sector = wbe->sector;
         free (wbe);

         ce_sample.sector = sector;

         lock_acquire (&cache_lock);
         h = hash_find (&cache_hash, &ce_sample.cache_elem);
         if (h == NULL) /* Already evictied. */
           {
             lock_release (&cache_lock);
             continue;
           }
         else 
           {
             ce = hash_entry (h, struct cache_entry, cache_elem);
             if (ce->modified)
               {
                 lock_acquire (&ce->use_num_lock);
                 ce->use_num++;
                 if (ce->use_num == 1)
                   ce->modified = false;
                 lock_release (&ce->use_num_lock);
                 lock_release (&cache_lock);
                 
                 /* Write to disk. */
                 disk_write (filesys_disk, sector, ce->buffer);

                 lock_acquire (&ce->use_num_lock);
                 ce->use_num--;
                 lock_release (&ce->use_num_lock);
               }
             else
               lock_release (&cache_lock);
             continue;
           }
       }
   }
}

static void
read_ahead (void *aux)
{
  struct cache_entry *ce;
  size_t sector = (size_t) aux;

  ce = get_cache_entry (sector, 0, false, false);
  decrease_use_num (ce);

  thread_exit ();
}

/* Find a cache entry with SECTOR. 
   If not found, then create a cache entry.
   If cache is full, evict one of cache entries.*/
struct cache_entry *
get_cache_entry (size_t sector, size_t next_sector, bool read_ahead_on, bool write)
{
  struct hash_elem *h, *h_;
  struct cache_entry *ce, *ce_out_, ce_sample;
 
  do 
    {
      lock_acquire (&cache_lock);
      /* Find a cache entry. */
      ce_sample.sector = sector;
      h = hash_find (&cache_hash, &ce_sample.cache_elem);
      h_ = hash_find (&removed_hash, &ce_sample.cache_elem);
      if (h != NULL) /* Cache entry is found. */
        {
          ce = hash_entry (h, struct cache_entry, cache_elem);
          lock_acquire (&ce->use_num_lock);
          ce->use_num++;
          lock_release (&ce->use_num_lock);

          while (ce->status == IO)
            cond_wait (&ce->io_finish, &cache_lock);
 
          if (ce->status == EXIST)
            {
 	      if (write)
                ce->modified = true;
              ce->accessed_time = timer_ticks ();
              lock_release (&cache_lock);

              /* Read ahead. */
              if (read_ahead_on)
                thread_create ("read_ahead", PRI_DEFAULT, read_ahead, (void *) next_sector);
              return ce;
            }
          else
            ASSERT (0);
        }
      else if (h_ != NULL) /* Cache entry is evicted. */
        {
          ce = hash_entry (h_, struct cache_entry, cache_elem);
          ce->wait_num++;

          while (ce->status == IO)
            cond_wait (&ce->io_finish, &cache_lock);
          
          if (ce->status == REMOVED)
            {
              ce->wait_num--;
              if (ce->wait_num == 0)
                free (ce);
              lock_release (&cache_lock);
              /* Go back to loop. */
              continue;
            }
          else
            ASSERT (0);
        }
      else /* Cannot find any cache entry. */
        break;

    } while (true);

  /* Create a cache entry. */
  ce = malloc (sizeof (struct cache_entry));
  if (ce == NULL)
    PANIC ("Insert cache fail: out of memory");

  ce->sector = sector;
  ce->modified = write ? true : false;
  ce->accessed_time = timer_ticks ();
  ce->status = IO;
  ce->wait_num = 0;
  ce->use_num = 1;
  cond_init (&ce->io_finish);
  lock_init (&ce->use_num_lock);

  /* Insert a cache entry. */
  hash_insert (&cache_hash, &ce->cache_elem);
  cache_cnt++;

  /* Evict a cache entry, */
  if (cache_cnt > MAX_CACHE_CNT)
    {
      ce_out = NULL;
      barrier ();
      hash_apply (&cache_hash, do_cache_evict);
      
      if ((ce_out_ = ce_out) == NULL)
        PANIC ("Insert cache fail: out of memory");
      
      /* Set status. */
      ce_out_->status = IO;

      /* Evict. */
      hash_delete (&cache_hash, &ce_out_->cache_elem);
      hash_insert (&removed_hash, &ce_out_->cache_elem);
      cache_cnt--;

      /* Write back to disk. */
      if (ce_out_->modified)
        {
          lock_release (&cache_lock);
          disk_write (filesys_disk, ce_out_->sector, ce_out_->buffer);\
        }
      else
        lock_release (&cache_lock);

      /* Free resources. */
      lock_acquire (&cache_lock);
      hash_delete (&removed_hash, &ce_out_->cache_elem);
      if (ce_out_->wait_num == 0)
        free (ce_out_);
      else
        {
          ce_out_->status = REMOVED;
          cond_broadcast (&ce_out_->io_finish, &cache_lock);
        }
    }
  
  /* Fetch data from disk. */
  lock_release (&cache_lock);
  disk_read (filesys_disk, sector, ce->buffer);

  /* Set new cache entry status to EXIST. */
  lock_acquire (&cache_lock);
  ce->status = EXIST;
  cond_broadcast (&ce->io_finish, &cache_lock);
  lock_release (&cache_lock);

  /* Read ahead. */
  if (read_ahead_on)
    thread_create ("read_ahead", PRI_DEFAULT, read_ahead, (void *) next_sector);

  return ce;
}

/* Decrease USE_NUM.
   The cache entry must exist. */
void
decrease_use_num (struct cache_entry *ce)
{
  ASSERT (ce != NULL);

  lock_acquire (&ce->use_num_lock);
  ASSERT (ce->use_num != 0);

  ce->use_num--;
  lock_release (&ce->use_num_lock);
}

/* Flush cache. 
   This funcion is called only in filesys_done (). */
void
flush_cache ()
{
  lock_acquire (&cache_lock); /* No one can get this lock. */
  hash_apply (&cache_hash, do_flush_cache);
}

/* Find if data exist in cache. 
   Unlike get_cache_entry () function, it does not fetch any data. */
bool
find_cached_data (size_t sector, void *data)
{
  struct hash_elem *h;
  struct cache_entry *ce, ce_sample;

  lock_acquire (&cache_lock);
  ce_sample.sector = sector;
  h = hash_find (&cache_hash, &ce_sample.cache_elem);
  if (h == NULL)
    {
      lock_release (&cache_lock);
      return false;
    }
  else
    {
      ce = hash_entry (h, struct cache_entry, cache_elem);
      ce->modified = false;
      if (data != NULL)
        memcpy (data, &ce->buffer, DISK_SECTOR_SIZE);
      lock_release (&cache_lock);
      return true;
    }
}

/* Hash function. */
static uint32_t
cache_hashing (const struct hash_elem *h, void *aux UNUSED)
{
  struct cache_entry *ce = hash_entry (h, struct cache_entry, cache_elem);
  return hash_bytes (&ce->sector, sizeof (ce->sector));
}

/* Hash comparison function. */
static bool
cache_less (const struct hash_elem *h1, const struct hash_elem *h2, void *aux UNUSED)
{
  struct cache_entry *ce1 = hash_entry (h1, struct cache_entry, cache_elem);
  struct cache_entry *ce2 = hash_entry (h2, struct cache_entry, cache_elem);

  return ce1->sector < ce2->sector;
}

/* Hash action function for write behind. */
static void
check_write_behind (struct hash_elem *h, void *aux UNUSED)
{
  struct cache_entry *ce = hash_entry (h, struct cache_entry, cache_elem);
  if (ce->modified)
    {
      struct write_back_entry *wbe = malloc (sizeof (struct write_back_entry));
      wbe->sector = ce->sector;
      list_push_back (&write_back_list, &wbe->write_back_elem);
    }
}

/* Hash action function for cache eviction. */
static void
do_cache_evict (struct hash_elem *h, void *aux UNUSED)
{
  struct cache_entry *ce = hash_entry (h, struct cache_entry, cache_elem);
  
  lock_acquire (&ce->use_num_lock);
  if (ce->status == EXIST && ce->use_num == 0 && (ce_out == NULL || ce->accessed_time < ce_out->accessed_time))
    ce_out = ce;
  lock_release (&ce->use_num_lock);
}

static void
do_flush_cache (struct hash_elem *h, void *aux UNUSED)
{
  struct cache_entry *ce = hash_entry (h, struct cache_entry, cache_elem);
  if (ce->modified == true)
    disk_write (filesys_disk, ce->sector, ce->buffer);

  free (ce);
}

