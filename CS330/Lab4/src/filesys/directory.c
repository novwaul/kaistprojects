#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#define DIR_ENTRY_SIZE 32

/* A directory. */
struct dir 
  {
    struct inode *inode;                /* Backing store. */
    struct lock *dir_lock;              /* Directory lock. */
    off_t pos;                          /* Current position. */
  };

/* A single directory entry. */
struct dir_entry 
  {
    disk_sector_t inode_sector;         /* Sector number of header. */
    char name[NAME_MAX + 1];            /* Null terminated file name. */
    bool in_use;                        /* In use or free? */
    bool is_dir;			/* Is directory or file? */
  };

/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool
dir_create (disk_sector_t sector, disk_sector_t prev_sector, size_t entry_cnt) 
{
  bool success = false;
  struct dir *dir; 
  if (inode_create (sector, entry_cnt * sizeof (struct dir_entry)))
    {
      dir = dir_open (inode_open (sector));
      dir_add (dir, "..", prev_sector, true);
      dir_add (dir, ".", sector, true);
      success = true;
      dir_close (dir);
    }

  return success;
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open (struct inode *inode) 
{
  struct dir *dir = calloc (1, sizeof *dir);
  if (inode != NULL && dir != NULL)
    {
      dir->inode = inode;
      dir->pos = 0;
      dir->dir_lock = inode_get_lock (inode);
      return dir;
    }
  else
    {
      inode_close (inode);
      free (dir);
      return NULL; 
    }
}

void
dir_set_pos (struct dir *dir, off_t pos)
{
  dir->pos = pos;
}

off_t
dir_get_pos (struct dir *dir)
{
  return dir->pos;
}

/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir *
dir_open_root (void)
{
  return dir_open (inode_open (ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir *
dir_reopen (struct dir *dir) 
{
  return dir_open (inode_reopen (dir->inode));
}

/* Destroys DIR and frees associated resources. */
void
dir_close (struct dir *dir) 
{
  if (dir != NULL)
    {
      inode_close (dir->inode);
      free (dir);
    }
}

/* Returns the inode encapsulated by DIR. */
struct inode *
dir_get_inode (struct dir *dir) 
{
  return dir->inode;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool
lookup (const struct dir *dir, const char *name,
        struct dir_entry *ep, off_t *ofsp) 
{
  struct dir_entry e;
  size_t ofs;
  
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  if (inode_is_removed (dir->inode))
    {
      return false;
    }

  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e) 
    if (e.in_use && !strcmp (name, e.name)) 
      {
        if (ep != NULL)
          *ep = e;
        if (ofsp != NULL)
          *ofsp = ofs;
        return true;
      }

  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool
dir_lookup (const struct dir *dir, const char *name,
            struct inode **inode, bool *is_dir) 
{
  struct dir_entry e;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);
 
  lock_acquire (dir->dir_lock); 
  if (lookup (dir, name, &e, NULL))
    {
      *inode = inode_open (e.inode_sector);
      
      if (is_dir != NULL)
        *is_dir = e.is_dir;
    }
  else
    *inode = NULL;
  lock_release (dir->dir_lock);
 
  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool
dir_add (struct dir *dir, const char *name, disk_sector_t inode_sector, bool is_dir) 
{
  struct dir_entry e;
  off_t ofs;
  bool success = false;
  
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  /* Check NAME for validity. */
  if (*name == '\0' || strlen (name) > NAME_MAX)
    return false;

  /* Check that NAME is not in use. */
  lock_acquire (dir->dir_lock);
  if (lookup (dir, name, NULL, NULL))
    goto done;

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.
     
     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */
  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e) 
    if (!e.in_use)
      break;

  /* Write slot. */
  e.in_use = true;
  strlcpy (e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  e.is_dir = is_dir;
  success = inode_write_at (dir->inode, &e, sizeof e, ofs) == sizeof e;

 done:
  lock_release (dir->dir_lock);
  return success;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool
dir_remove (struct dir *dir, const char *name) 
{
  struct dir_entry e;
  struct inode *inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  /* Find directory entry. */
  lock_acquire (dir->dir_lock);

  if (strcmp (name, "..") == 0 || strcmp (name, ".") == 0)
    goto done; 

  if (!lookup (dir, name, &e, &ofs))
    goto done; 
    
  
  /* Open inode. */
  inode = inode_open (e.inode_sector);
  if (inode == NULL)
    goto done;

  /* If directory, check it is empty. */
  if (e.is_dir)
    {
      off_t ofs_t;
      struct dir_entry e_;
      lock_acquire (inode_get_lock (inode));
      for (ofs_t = 0; inode_read_at (inode, &e_, sizeof e_, ofs_t) == sizeof e_;
           ofs_t += sizeof e_) 
        if (e_.in_use && strcmp (e_.name, "..") != 0 && strcmp (e_.name, ".") != 0)
          {
            lock_release (inode_get_lock (inode));
            goto done;
          }
      lock_release (inode_get_lock (inode));
    }

  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at (dir->inode, &e, sizeof e, ofs) != sizeof e) 
    goto done;

  /* Remove inode. */
  inode_remove (inode);
  success = true;
      
  done:
   inode_close (inode);
   lock_release (dir->dir_lock);
   return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool
dir_readdir (struct dir *dir, char name[NAME_MAX + 1])
{
  struct dir_entry e;

  lock_acquire (dir->dir_lock);
  while (inode_read_at (dir->inode, &e, sizeof e, dir->pos) == sizeof e) 
    {
      dir->pos += sizeof e;
      if (e.in_use && strcmp (e.name, "..") != 0 && strcmp (e.name, ".") != 0)
        {
          strlcpy (name, e.name, NAME_MAX + 1);
          lock_release (dir->dir_lock);
          return true;
        } 
    }
  lock_release (dir->dir_lock);
  return false;
}
