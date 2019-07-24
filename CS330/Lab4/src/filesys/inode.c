#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "filesys/cache.h"
#include "threads/malloc.h"
#include "filesys/inode.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44
#define MAX_SECTORS 16384
#define MAX_CAPACITY 127
#define INODE_MAX_DC 12
#define INODE_MAX_IDC 2

/* On-disk inode.
   Must be exactly DISK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    disk_sector_t direct [INODE_MAX_DC];  	        /* Direct data sectors. */
    disk_sector_t indirect [INODE_MAX_IDC];		/* Indirect data sectors. */
    disk_sector_t double_indirect;     	 		/* Double indirect data sector. */
    uint32_t direct_idx;				/* Direct index. */
    uint32_t indirect_idx;				/* Indirect index. */
    uint32_t sectors;					/* # of total sectors. */
    uint32_t data_sectors;				/* # of data containing direct sectors. */
    off_t length;                       		/* File size in bytes. */
    unsigned magic;                     		/* Magic number. */
    uint32_t unused[107];               		/* Not used. */
  };

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    disk_sector_t sector;               /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
    struct lock inode_lock;		/* Inode member update lock. 
                                           OPEN_CNT, REMOVED, DENY_WRITE_CNT, DATA etc. */
    struct lock inode_write_lock;	/* Inode write extend lock. */
    struct lock dir_lock;		/* Directory update lock.  
                                           //Owned by filesys/directory.c file. */
  };

struct indirect_inode_disk
  {
    uint32_t direct_idx;
    disk_sector_t direct [MAX_CAPACITY];
  };

struct double_indirect_inode_disk
  {
    uint32_t indirect_idx;
    disk_sector_t indirect [MAX_CAPACITY];
  };

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;
static struct lock inode_list_lock;

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, DISK_SECTOR_SIZE);
}

static void remove_disk_sectors (struct inode_disk *disk_inode);
static bool extend_file (struct inode_disk *disk_inode, size_t data_sectors);

struct lock *
inode_get_lock (struct inode *inode)
{
  return &inode->dir_lock;
}

bool
inode_is_removed (struct inode *inode)
{
  bool ret;
  lock_acquire (&inode->inode_lock);
  ret = inode->removed;
  lock_release (&inode->inode_lock);

  return ret;
}

/* Returns the disk sector that contains byte offset POS within
   INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static disk_sector_t
byte_to_sector (struct inode *inode, off_t pos, off_t extended_length) 
{
  ASSERT (inode != NULL);
  bool valid = false;

  lock_acquire (&inode->inode_lock);
  if (extended_length > 0)
    valid = pos < extended_length;
  else
    valid = pos < inode->data.length;
  lock_release (&inode->inode_lock);

  if (valid)
    {
      uint32_t indirect_idx, logical_idx = pos / DISK_SECTOR_SIZE;
      disk_sector_t sector, indirect_sector;
      struct indirect_inode_disk *data;
      struct double_indirect_inode_disk *indirect_data;
      struct cache_entry *ce;

      if (logical_idx < INODE_MAX_DC) /* Direct block case. */
        { 
          sector = inode->data.direct [logical_idx];
          return sector;
        }
      else if (logical_idx < INODE_MAX_DC + MAX_CAPACITY) /* Indirect block case 1. */
        {
          ce = get_cache_entry (inode->data.indirect [0], 0, false, false);
          data = (struct indirect_inode_disk *) &ce->buffer;
          sector = data->direct [logical_idx - INODE_MAX_DC];
          decrease_use_num (ce);
          return sector;
        }
      else if (logical_idx < INODE_MAX_DC + INODE_MAX_IDC * MAX_CAPACITY) /* Indirect block case 2. */
        {
          ce = get_cache_entry (inode->data.indirect [1], 0, false, false);
          data = (struct indirect_inode_disk *) &ce->buffer;
          sector = data->direct [logical_idx - INODE_MAX_DC - MAX_CAPACITY];
          decrease_use_num (ce);
          return sector;
        }
      else /* Double indirect block case. */
        {
          ce = get_cache_entry (inode->data.double_indirect, 0, false, false);
          indirect_data = (struct double_indirect_inode_disk *) &ce->buffer;

          indirect_idx = (logical_idx - INODE_MAX_DC - INODE_MAX_IDC * MAX_CAPACITY) / MAX_CAPACITY;
          indirect_sector = indirect_data->indirect [indirect_idx];
          decrease_use_num (ce);

          ce = get_cache_entry (indirect_sector, 0, false, false);
          data = (struct indirect_inode_disk *) &ce->buffer;
          sector = data->direct [logical_idx - INODE_MAX_DC - INODE_MAX_IDC * MAX_CAPACITY - indirect_idx * MAX_CAPACITY];
          decrease_use_num (ce);

          return sector;
        }
    }
  else
    return -1;
}

/* Initializes the inode module. */
void
inode_init (void) 
{
  list_init (&open_inodes);
  lock_init (&inode_list_lock);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   disk.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (disk_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == DISK_SECTOR_SIZE);

  if (sector == (disk_sector_t )-1) 
    return false;

  size_t data_sectors = bytes_to_sectors (length);
  if (data_sectors > MAX_SECTORS)
    return false;

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      disk_inode->direct_idx = 0;
      disk_inode->indirect_idx = 0;
      disk_inode->sectors = 1;
      disk_inode->data_sectors = 0;
      success = extend_file (disk_inode, data_sectors);

      if (!success)
        remove_disk_sectors (disk_inode);
      else
        disk_write (filesys_disk, sector, disk_inode);
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (disk_sector_t sector) 
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  lock_acquire (&inode_list_lock);
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector) 
        {
          inode_reopen (inode);
          lock_release (&inode_list_lock);
          return inode; 
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    {
      lock_release (&inode_list_lock);
      return NULL;
    }

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  lock_init (&inode->inode_lock);
  lock_init (&inode->inode_write_lock);
  lock_init (&inode->dir_lock);

  /* Fetch data. */
  lock_acquire (&inode->inode_lock); /* Prevent reading DATA before fetching it. */
  lock_release (&inode_list_lock);
  disk_read (filesys_disk, inode->sector, &inode->data);
  lock_release (&inode->inode_lock);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    {
      lock_acquire (&inode->inode_lock);
      inode->open_cnt++;
      lock_release (&inode->inode_lock);
    }
  return inode;
}

/* Returns INODE's inode number. */
disk_sector_t
inode_get_inumber (struct inode *inode)
{
  disk_sector_t sector;
  lock_acquire (&inode->inode_lock);
  sector = inode->sector;
  lock_release (&inode->inode_lock);
  return sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  lock_acquire (&inode_list_lock);
  lock_acquire (&inode->inode_lock);

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);
      lock_release (&inode_list_lock);
 
      /* Deallocate blocks if removed. */
      if (inode->removed) 
        {
	  remove_disk_sectors (&inode->data);
          free_map_release (inode->sector, 1);
        }
      else
        {
          disk_write (filesys_disk, inode->sector, &inode->data);
        }

      free (inode);
    } /* if ends. */
  else
    {
      lock_release (&inode->inode_lock);
      lock_release (&inode_list_lock);
    }

}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  lock_acquire (&inode->inode_lock);
  inode->removed = true;
  lock_release (&inode->inode_lock);
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  struct cache_entry *ce;
  
  while (size > 0) 
    {
      /* Disk sector to read, starting byte offset within sector. */ 
      int sector_ofs = offset % DISK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = DISK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;
      else if (chunk_size >= sector_left)
        {
          disk_sector_t sector_idx = byte_to_sector (inode, offset, 0); 
          disk_sector_t next_sector_idx = byte_to_sector (inode, offset + chunk_size, 0);
          ce = get_cache_entry (sector_idx, next_sector_idx, (next_sector_idx != (disk_sector_t) -1), false);
        }
      else
        {
          disk_sector_t sector_idx = byte_to_sector (inode, offset, 0);
          ce = get_cache_entry (sector_idx, 0, false, false);
        }

      memcpy (buffer + bytes_read, ce->buffer + sector_ofs, chunk_size);
      decrease_use_num (ce);
     
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t extended_length = 0, bytes_written = 0;
  bool extending = false;
  
  lock_acquire (&inode->inode_lock);
  if (inode->deny_write_cnt)
    {
      lock_release (&inode->inode_lock);
      return 0;
    }
  lock_release (&inode->inode_lock);
  

  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      int sector_ofs = offset % DISK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = (extending ? extended_length : inode_length (inode)) - offset;
      int sector_left = DISK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        {
          lock_acquire (&inode->inode_write_lock);
          lock_acquire (&inode->inode_lock);
          off_t len = inode->data.length;
          uint32_t cur_sectors = inode->data.data_sectors;
          lock_release (&inode->inode_lock);          

          if (len - offset > 0)
            {
              lock_release (&inode->inode_write_lock);
              continue;
            }
          /* Extend file. */
          uint32_t add_sectors = bytes_to_sectors (offset + size) - cur_sectors;
          if (add_sectors == 0)
            {
              extended_length = offset + size;
            }
          else
            {
              bool success = extend_file (&inode->data, add_sectors);
              if (success)
                {
                  extended_length = offset + size;
                }
              else
                {
                  extended_length = inode->data.data_sectors * DISK_SECTOR_SIZE;
                }
            }
          extending = true;
          continue;
        }

      disk_sector_t sector_idx = sector_idx = byte_to_sector (inode, offset, extended_length);
      
      struct cache_entry *ce = get_cache_entry (sector_idx, 0, false, true);
      if (sector_ofs == 0 && chunk_size == sector_left && chunk_size < DISK_SECTOR_SIZE)
        memset (ce->buffer + sector_ofs, 0, chunk_size);
      else
        memcpy (ce->buffer + sector_ofs, buffer + bytes_written, chunk_size);
      decrease_use_num (ce);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }

  if (extending)
    {
      lock_acquire (&inode->inode_lock);
      inode->data.length = extended_length;
      lock_release (&inode->inode_lock);
      lock_release (&inode->inode_write_lock);
    }

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  lock_acquire (&inode->inode_lock);
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  lock_release (&inode->inode_lock);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  lock_acquire (&inode->inode_lock);
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
  lock_release (&inode->inode_lock);
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (struct inode *inode)
{ 
  off_t len;
  lock_acquire (&inode->inode_lock);
  len = inode->data.length;
  lock_release (&inode->inode_lock);
  return len;
}

static bool
extend_file (struct inode_disk *disk_inode, size_t data_sectors)
{
  struct inode_disk *zeros = calloc (1, sizeof *zeros);
  uint32_t written_sectors = 0;

  if (zeros == NULL)
    return false;

  while (written_sectors < data_sectors)
    {
      if (disk_inode->direct_idx != INODE_MAX_DC) /* Direct block case. */
        {
          /* Error. */
          if (!free_map_allocate (1, &disk_inode->direct [disk_inode->direct_idx]))
            {
              free (zeros);
              return false; 
            }
          /* Allocate direct block. */
          disk_write (filesys_disk, disk_inode->direct [disk_inode->direct_idx], zeros);
          disk_inode->direct_idx++;
          disk_inode->sectors++;
          disk_inode->data_sectors++;
          written_sectors++;
        }
      else if (disk_inode->indirect_idx != INODE_MAX_IDC) /* Indirect block case. */
        {
          struct indirect_inode_disk *indirect_disk;
          struct cache_entry *ce = NULL;
          bool record = false;
          if (disk_inode->data_sectors == INODE_MAX_DC || disk_inode->data_sectors == INODE_MAX_DC + MAX_CAPACITY)
            {
              /* Allocate new indirect block. */
              indirect_disk = calloc (1, sizeof *indirect_disk);
              /* Error */
              if (indirect_disk == NULL)
                {
                  free (zeros);
                  return false;
                }
              if (!free_map_allocate (1, &disk_inode->indirect [disk_inode->indirect_idx]))
                {
                  free (zeros);
                  free (indirect_disk);
                  return false;
                }
              /* Setting. */
              disk_inode->sectors++;
              indirect_disk->direct_idx = 0;
              record = true;
            }
          else
            {
              /* Find cached indirect block. */
              ce = get_cache_entry (disk_inode->indirect [disk_inode->indirect_idx], 0, false, true);
              indirect_disk = (struct indirect_inode_disk *) &ce->buffer;
            }
          /* Allocate direct blocks. */
          while (written_sectors < data_sectors && indirect_disk->direct_idx < MAX_CAPACITY)
            {
              if (!free_map_allocate (1, &indirect_disk->direct [indirect_disk->direct_idx]))
                {
                  /* Error. */
                  if (record)
                    {
                      disk_write (filesys_disk, disk_inode->indirect [disk_inode->indirect_idx], indirect_disk);
                      free (indirect_disk);
                    }
                  else
                    decrease_use_num (ce);

                  free (zeros);
                  return false;
                } 
              disk_write (filesys_disk, indirect_disk->direct [indirect_disk->direct_idx], zeros);
              indirect_disk->direct_idx++;
              disk_inode->sectors++;
              disk_inode->data_sectors++;
              written_sectors++;
            }
          uint32_t direct_idx = indirect_disk->direct_idx;
          if (record)
            {
              /* Record indirect block to disk. */
              disk_write (filesys_disk, disk_inode->indirect [disk_inode->indirect_idx], indirect_disk);
              free (indirect_disk);
            }
          else
            decrease_use_num (ce);
          if (direct_idx == MAX_CAPACITY)
            disk_inode->indirect_idx++;
        }
      else /* Double indirect block case. */
        {
          struct double_indirect_inode_disk *double_disk; 
          struct cache_entry *ce_double = NULL;
          bool record_double = false;
          if (disk_inode->data_sectors == INODE_MAX_DC + INODE_MAX_IDC * MAX_CAPACITY)
            {
              /* Allocate double indirect block. */
              double_disk = calloc (1, sizeof *double_disk);
              /* Error. */
              if (double_disk == NULL)
                {
                  free (zeros);
                  return false;
                }
              if (!free_map_allocate (1, &disk_inode->double_indirect))
                {
                  free (zeros);
                  free (double_disk);
                  return false;
                }
              /* Setting. */
              disk_inode->sectors++;
              double_disk->indirect_idx = 0;
              record_double = true;
            }
          else
            {
              /* Find cached double indirect block. */    
              ce_double = get_cache_entry (disk_inode->double_indirect, 0, false, true);
              double_disk = (struct double_indirect_inode_disk *) &ce_double->buffer;
            }
          while (written_sectors < data_sectors && double_disk->indirect_idx < MAX_CAPACITY)
            { 
              struct cache_entry *ce_indirect = NULL;
              struct indirect_inode_disk *indirect_disk;
              bool record_indirect = false;
              if (disk_inode->data_sectors == INODE_MAX_DC + (INODE_MAX_IDC + double_disk->indirect_idx) * MAX_CAPACITY)
                {
                  /* Allocate indirect_block. */
                  indirect_disk = calloc (1, sizeof *indirect_disk);
                  if (disk_inode->sectors + 1 > MAX_SECTORS || indirect_disk == NULL || 
                      !free_map_allocate (1, &double_disk->indirect [double_disk->indirect_idx]))
                    {
                      if (record_double)
                        {
                          disk_write (filesys_disk, disk_inode->double_indirect, double_disk);
                          free (double_disk);
                        }
                      else
                        decrease_use_num (ce_double);
                      if (indirect_disk != NULL)
                        free (indirect_disk);

                      free (zeros);
                      return false;
                    }
                  /* Setting. */
                  indirect_disk->direct_idx = 0;
                  disk_inode->sectors++;
                  record_indirect = true;
                }
              else
                {
                  /* Find cached indiect block. */
                  ce_indirect = get_cache_entry (double_disk->indirect [double_disk->indirect_idx], 0, false, true);
                  indirect_disk = (struct indirect_inode_disk *) &ce_indirect->buffer;
                }
              /* Allocate direct block. */
              while (written_sectors < data_sectors && indirect_disk->direct_idx < MAX_CAPACITY)
                {
                  if (disk_inode->sectors + 1 > MAX_SECTORS ||
                      !free_map_allocate (1, &indirect_disk->direct [indirect_disk->direct_idx]))
                    {
                      if (record_indirect)
                        {
                          disk_write (filesys_disk, double_disk->indirect [double_disk->indirect_idx], indirect_disk);
                          free (indirect_disk);
                        }
                      else
                        decrease_use_num (ce_indirect);
                      if (record_double)
                        {
	                  disk_write (filesys_disk, disk_inode->double_indirect, double_disk);
                          free (double_disk);
                        }
                      else
                        decrease_use_num (ce_double);
                   
                      free (zeros);
                      return false;
                    } 
                  disk_write (filesys_disk, indirect_disk->direct [indirect_disk->direct_idx], zeros);
                  indirect_disk->direct_idx++;
                  disk_inode->sectors++;
                  disk_inode->data_sectors++;
                  written_sectors++;
                }
              uint32_t direct_idx = indirect_disk->direct_idx;    
              if (record_indirect)
                {
                  /* Record indirect block. */
                  disk_write (filesys_disk, double_disk->indirect [double_disk->indirect_idx], indirect_disk);
                  free (indirect_disk);
                }
              else
                decrease_use_num (ce_indirect);
              if (direct_idx == MAX_CAPACITY)
                double_disk->indirect_idx++;

            }
          if (record_double)
            {
              /* Record double indirect block. */
	      disk_write (filesys_disk, disk_inode->double_indirect, double_disk);
              free (double_disk);
            }
          else
            decrease_use_num (ce_double);
        }
    }
  free (zeros);
  return true;
}

static void
remove_disk_sectors (struct inode_disk *disk_inode)
{
  if (disk_inode->sectors > 1)
    {
      if (disk_inode->data_sectors > INODE_MAX_DC + INODE_MAX_IDC * MAX_CAPACITY)
        {
          struct double_indirect_inode_disk double_disk;
          disk_sector_t double_sector, indirect_sector, direct_sector;
          double_sector = disk_inode->double_indirect;
          
          if (!find_cached_data (double_sector, &double_disk))
            disk_read (filesys_disk, double_sector, &double_disk);
          while (double_disk.indirect_idx != 0)
            {
              struct indirect_inode_disk indirect_disk;
              indirect_sector = double_disk.indirect [double_disk.indirect_idx - 1];

              if (!find_cached_data (indirect_sector, &indirect_disk))
                disk_read (filesys_disk, indirect_sector, &indirect_disk);
              while (indirect_disk.direct_idx != 0)
                {
                  direct_sector = indirect_disk.direct [indirect_disk.direct_idx - 1];
                  free_map_release (direct_sector, 1);
                  find_cached_data (direct_sector, NULL);
                  indirect_disk.direct_idx--;
                  disk_inode->data_sectors--;
                }
              free_map_release (indirect_sector, 1);
              double_disk.indirect_idx--;
            }
          free_map_release (disk_inode->double_indirect, 1);
        }
      while (INODE_MAX_DC < disk_inode->data_sectors && 
             disk_inode->data_sectors <= INODE_MAX_DC + INODE_MAX_IDC * MAX_CAPACITY)
        {
          struct indirect_inode_disk indirect_disk;
          disk_sector_t direct_sector, indirect_sector;
          if (disk_inode->data_sectors == INODE_MAX_DC + MAX_CAPACITY ||
              disk_inode->data_sectors == INODE_MAX_DC + INODE_MAX_IDC * MAX_CAPACITY)
            disk_inode->indirect_idx--;

          indirect_sector = disk_inode->indirect [disk_inode->indirect_idx];
          if (!find_cached_data (indirect_sector, &indirect_disk))
            disk_read (filesys_disk, indirect_sector, &indirect_disk);
          while (indirect_disk.direct_idx != 0)
            {
              direct_sector = indirect_disk.direct [indirect_disk.direct_idx - 1];
              free_map_release (direct_sector, 1);
              find_cached_data (direct_sector, NULL);
              indirect_disk.direct_idx--;
              disk_inode->data_sectors--;
            }
          free_map_release (indirect_sector, 1);
        }
      while (0 < disk_inode->data_sectors && disk_inode->data_sectors <= INODE_MAX_DC)
        {
          disk_sector_t sector = disk_inode->direct [disk_inode->direct_idx - 1];
          free_map_release (sector, 1);
          find_cached_data (sector, NULL);
          disk_inode->direct_idx--;
          disk_inode->data_sectors--;
        }

    }
}
