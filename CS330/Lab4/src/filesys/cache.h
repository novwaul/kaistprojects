#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include "devices/disk.h"
#include "threads/synch.h"
#include "filesys/inode.h"
#include <hash.h>

#define IO 0
#define EXIST 1
#define REMOVED 2

struct cache_entry
{
  uint8_t buffer [DISK_SECTOR_SIZE];
  struct hash_elem cache_elem;
  struct condition io_finish;
  struct lock use_num_lock;
  bool modified;
  int64_t accessed_time;
  uint32_t status;
  uint32_t wait_num;
  uint32_t use_num;
  uint32_t type;
  size_t sector;
  struct inode *inode;
};

void cache_init (void);
void flush_cache (void);
void decrease_use_num (struct cache_entry *ce);
bool find_cached_data (size_t sector, void *data);
struct cache_entry *get_cache_entry (size_t sector, size_t next_sector, bool read_ahead_on, bool write);

#endif /* filesys/cache.h */
