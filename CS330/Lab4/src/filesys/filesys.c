#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "threads/thread.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/cache.h"
#include "devices/disk.h"

#define MAX_PATH 256

/* The disk that contains the file system. */
struct disk *filesys_disk;

static void do_format (void);
static struct dir* do_parse (const char *path, char *name);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  filesys_disk = disk_get (0, 1);
  if (filesys_disk == NULL)
    PANIC ("hd0:1 (hdb) not present, file system initialization failed");

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  flush_cache ();
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *path, off_t initial_size) 
{
  disk_sector_t inode_sector = 0;
  char file_name [NAME_MAX + 1];
  struct dir *dir = do_parse (path, file_name);

  bool success = (dir != NULL
                  && !inode_is_removed (dir_get_inode (dir))
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size)
                  && dir_add (dir, file_name, inode_sector, false));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *path)
{
  char file_name [NAME_MAX + 1];
  struct dir *dir;
  struct inode *inode = NULL;
  struct file *file;
  bool is_dir;

  if (*path == '/' && *(path+1) == '\0')
    {
      file = file_open (inode_open (ROOT_DIR_SECTOR), true);
    }
  else
    {
      dir = do_parse (path, file_name);

      if (dir != NULL)
        dir_lookup (dir, file_name, &inode, &is_dir);
      dir_close (dir);

      file = file_open (inode, is_dir);
    }
  return file;
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *path) 
{
  char file_name [NAME_MAX + 1];
  struct dir *dir = do_parse (path, file_name);

  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir); 

  return success;
}

bool
filesys_chdir (const char *path)
{
  char dir_name [NAME_MAX + 1];
  struct dir *old_dir;
  struct inode *inode = NULL;
  struct thread *t = thread_current ();
  bool is_dir = false;

  if (*path == '/' && *(path + 1) == '\0')
    {
      dir_close (t->dir);
      t->dir = dir_open (inode_open (ROOT_DIR_SECTOR));
      t->cur_dir_sector = ROOT_DIR_SECTOR;
    }
  else
    {
      old_dir = do_parse (path, dir_name);

      if (old_dir == NULL)
        {
          return false;
        }

      if (!dir_lookup (old_dir, dir_name, &inode, &is_dir))
        {
          dir_close (old_dir);
          return false;
        }
      else if (!is_dir)
        {
          dir_close (old_dir);
          inode_close (inode);
          return false;
        }

      dir_close (old_dir);
      dir_close (t->dir);
      t->dir = dir_open (inode);
      t->cur_dir_sector = inode_get_inumber (inode);
    }
  
  return true;
}

bool
filesys_mkdir (const char *path)
{
  disk_sector_t inode_sector = 0;
  char dir_name [NAME_MAX + 1];
  struct dir *dir = do_parse (path, dir_name);

  bool success = (dir != NULL
		  && !inode_is_removed (dir_get_inode (dir))
                  && free_map_allocate (1, &inode_sector)
                  && dir_create (inode_sector, inode_get_inumber (dir_get_inode (dir)), 2)
                  && dir_add (dir, dir_name, inode_sector, true));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);

  return success;
}


/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}

/* Parsing path and find out the last directory sector. 
   Also, it finds out file name and store it in NAME. */
static struct dir*
do_parse (const char *path, char *name)
{
  size_t len;
  const char *cur, *prev;
  char name_t [NAME_MAX + 1];
  struct dir *dir, *dir_ret = NULL;
  bool is_dir;
  struct inode *inode;

  if (path == NULL)
    goto done;

  dir = dir_open (inode_open (thread_current ()->cur_dir_sector));
  prev = path;
  cur = strchr (path, '/');
 
  while (cur != NULL)
    {
      /* Open directory. */ 
      if (prev == cur)
        {
          dir_close (dir);
          dir = dir_open_root ();
        }
      else
        {
          if (NAME_MAX < (len = (size_t) (cur - prev)))
            {
              dir_close (dir);
              goto done;
            }
          strlcpy (name_t, prev, len + 1); 
          if (!dir_lookup (dir, name_t, &inode, &is_dir))
            {
              dir_close (dir);
              goto done;
            }
          else if (!is_dir)
            {
              dir_close (dir);
              inode_close (inode);
              goto done;
            }
          dir_close (dir);
          dir = dir_open (inode);
        }  

      /* Advance. */
      prev = cur + 1;
      cur = strchr (cur + 1, '/');
   
      /* Ignore multiple slashes. */
      while (prev == cur)
        {
          prev = cur + 1;
          cur = strchr (cur + 1, '/');
          /* Check if ends with slash. */
          if (cur == NULL && *prev == '\0')
            {
              dir_close (dir);
              goto done;
            }
          else if (cur == NULL)
            break;
        }
    }

  if (strnlen (prev, NAME_MAX + 1) > NAME_MAX)
    {
      dir_close (dir);
      goto done;
    }

  strlcpy (name, prev, NAME_MAX + 1);
  dir_ret = dir;

 done:  
  return dir_ret;
}
