#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include <list.h>
#include "devices/input.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/ctrlsp.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/directory.h"
#include "filesys/inode.h"
#ifdef VM
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include <hash.h>
#endif

#define DEFAULT_WRITE  128
#define MAX_PATH 200

static void syscall_handler (struct intr_frame *);
static void kill_process (int exit_code);
static bool validate (void *p, bool on, void *esp);
static void sys_halt (void *esp UNUSED, struct intr_frame *f UNUSED);
static void sys_exit (void *esp, struct intr_frame *f UNUSED);
static void sys_exec (void *esp, struct intr_frame *f);
static void sys_wait (void *esp, struct intr_frame *f);
static void sys_create (void *esp, struct intr_frame *f);
static void sys_remove (void *esp, struct intr_frame *f);
static void sys_open (void *esp, struct intr_frame *f);
static void sys_filesize (void *esp, struct intr_frame *f);
static void sys_read (void *esp, struct intr_frame *f);
static void sys_write (void *esp, struct intr_frame *f);
static void sys_seek (void *esp, struct intr_frame *f UNUSED);
static void sys_tell (void *esp, struct intr_frame *f);
static void sys_close (void *esp, struct intr_frame *f UNUSED);
static void sys_mmap (void *esp, struct intr_frame *f);
static void sys_munmap (void *esp, struct intr_frame *f UNUSED);
static void sys_chdir (void *esp, struct intr_frame *f);
static void sys_mkdir (void *esp, struct intr_frame *f);
static void sys_readdir (void *esp, struct intr_frame *f);
static void sys_isdir (void *esp, struct intr_frame *f);
static void sys_inumber (void *esp, struct intr_frame *f);
static bool check_string (void *ptr);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  void *esp = f->esp;
  int idx;
  struct thread *t = thread_current ();

  if (!validate (esp, false, NULL))
    kill_process (ERR);
  else
    {
      t->in_syscall = true;
      idx = *(int *) esp;
      switch (idx)
        {
        case SYS_HALT: 
          sys_halt (esp, f);
          break; 
        case SYS_EXIT:
          sys_exit (esp, f);
          break;
        case SYS_EXEC:
          sys_exec (esp, f);
          break;
        case SYS_WAIT:
          sys_wait (esp, f);
          break;
        case SYS_CREATE:
          sys_create (esp, f);
          break;
        case SYS_REMOVE:
          sys_remove (esp, f);
          break;
        case SYS_OPEN:
          sys_open (esp, f);
          break;
        case SYS_FILESIZE:
          sys_filesize (esp, f);
          break;
        case SYS_READ:
          sys_read (esp, f);
          break;
        case SYS_WRITE:
          sys_write (esp, f);
          break;
        case SYS_SEEK:
          sys_seek (esp, f);
          break;
        case SYS_TELL:
          sys_tell (esp, f);
          break;
        case SYS_CLOSE:
          sys_close (esp, f);
	  break;
        case SYS_MMAP:
          sys_mmap (esp, f);
          break;
        case SYS_MUNMAP:
	  sys_munmap (esp, f);
	  break;
        case SYS_CHDIR:
          sys_chdir (esp, f);
          break;
        case SYS_MKDIR:
          sys_mkdir (esp, f);
          break;
        case SYS_READDIR:
          sys_readdir (esp, f);
          break;
        case SYS_ISDIR:
          sys_isdir (esp, f);
          break;
        case SYS_INUMBER:
          sys_inumber (esp, f);
          break;
        default: break;
        };
      t->in_syscall = false;
#ifdef VM
      unfix (esp);
#endif
      return;
    }
}

/* Validate user pointer. */
static bool
validate (void *p, bool on, void *esp)
{
  if (p == NULL)
    return false;
  else if (!is_user_vaddr (p))
    return false;
  else if (p < (void *) USER_BASE)
    return false;
  else if (!pagedir_get_page (thread_current ()->pagedir, p))
    {
       bool success = false;
#ifdef VM
       if (find_spte (p) != NULL)
         {
           success = swap_in (p, true);
         }
      else if (esp != NULL &&
               on &&
               p >= (void *) (esp - 32) &&
               thread_current ()->process_stack_page_count < MAX_STACK_CNT)
         {
           /* Stack grow */
           stack_grow (p, true);
           success = true;    
         }
#endif
       return success;
    }
  else 
    return true;
}

static bool
check_string (void *ptr)
{
  char *input = (char *) ptr;
  uint32_t i;
  
  for (i = 0; i < MAX_PATH; i++)
    {
      if (!validate (input, true, NULL))
        return false;
      if (*input == '\0')
        return true;
      else
        input++;
    }

  /* Too long. */
  return false;
}

/* Kill a process. */
static void
kill_process (int exit_code)
{
  struct thread *curr = thread_current ();

  curr->exit_code = exit_code;

  /* Kill. */
  thread_exit ();
}

static void
sys_chdir (void *esp, struct intr_frame *f)
{
  char *input, *path;
  bool success = false; 

  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (input, char *, esp);

  path = malloc (MAX_PATH);
  if (path != NULL)
    {
      if (check_string (input))
        {
          strlcpy (path, input, MAX_PATH);
          success = filesys_chdir (path);
        }
      free (path);
    }

  f->eax = success;
}

static void
sys_mkdir (void *esp, struct intr_frame *f)
{
  char *input, *path;
  bool success = false;

  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (input, char *, esp);

  path = malloc (MAX_PATH);
  if (path != NULL)
    {
      if (check_string (input))
        {
          strlcpy (path, input, MAX_PATH);
          success = filesys_mkdir (path);
        }
      free (path);
    }

  f->eax = success;
}

static void
sys_readdir (void *esp, struct intr_frame *f)
{
  int fd, i;
  char *temp, *name;
  struct file_info *finfo;
  struct file *file = NULL;
  struct list_elem *e;
  struct thread *t = thread_current ();
  bool success = false;
  struct dir *dir;
  
  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);

  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (name, char *, esp);
  
  temp = name;

  for (i = 0; i < NAME_MAX + 1; i++)
    if (!validate (temp++, true, esp))
      kill_process (ERR);

  /* Find file. */
  for (e = list_begin (&t->file_list); e != list_end (&t->file_list); e = list_next (e))
    {
      finfo = list_entry (e, struct file_info, elem_file);
      if (finfo->fd == fd)
        {
          file = finfo->file;
          if (file_is_dir (file))
            {
              dir = dir_open (file_get_inode (file));
              dir_set_pos (dir, file_get_pos (file));
              success = dir_readdir (dir, name);
              file_set_pos (file, dir_get_pos (dir));
              free (dir);
            }
          break;
        }
    }
  
  f->eax = success;
}

static void
sys_isdir (void *esp, struct intr_frame *f)
{
  int fd;
  struct list_elem *e;
  struct file_info *finfo;
  struct file * file;
  struct thread *t = thread_current ();
  bool ret = false;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);

  for (e = list_begin (&t->file_list); e != list_end (&t->file_list); e = list_next (e))
    {
      finfo = list_entry (e, struct file_info, elem_file);
      if (finfo->fd == fd)
        {
          file = finfo->file;
          if (file_is_dir (file))
            ret = true;
          break;
        }
    }
  f->eax = ret;
}

static void
sys_inumber (void *esp, struct intr_frame *f)
{
  int fd;
  struct list_elem *e;
  struct file_info *finfo;
  struct file * file;
  struct thread *t = thread_current ();
  uint32_t sector = -1;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);

  for (e = list_begin (&t->file_list); e != list_end (&t->file_list); e = list_next (e))
    {
      finfo = list_entry (e, struct file_info, elem_file);
      if (finfo->fd == fd)
        {
          file = finfo->file;
          sector = inode_get_inumber (file_get_inode (file));
          break;
        }
    }
  f->eax = sector;
}

static void
sys_mmap (void *esp, struct intr_frame *f)
{
#ifdef VM
  int fd, i;
  void *addr;
  struct list_elem *e;
  struct thread *t = thread_current ();
  struct page_table_entry *spte;
  struct file_info *finfo;
  struct file *file = NULL, *new_file;
  off_t size, read_size, of;
  struct mmap_info *minfo;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);
  if (fd == 0 || fd == 1) 
    {
      f->eax = MAPID_ERROR;
      return;
    }

  MOV (esp, void **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (addr, void *, esp);
  if (addr == NULL || 
      !is_user_vaddr (addr) || 
      addr < (void *) USER_BASE ||
      pg_ofs (addr) != 0)
    {
      f->eax = MAPID_ERROR;
      return;
    }

  /* Find file. */
  for (e = list_begin (&t->file_list); e != list_end (&t->file_list); e = list_next (e))
    {
      finfo = list_entry (e, struct file_info, elem_file);
      if (finfo->fd == fd)
        {
          file = finfo->file;
          break;
        }
    }
  /* File does not exist. */
  if (e == list_end (&t->file_list))
    {
      f->eax = MAPID_ERROR;
      return;
    }
  /* File size is  0. */
  if ((size = file_length (file)) == 0)
    {
      f->eax = MAPID_ERROR;
      return;
    }

  /* Check overlapping. */
  for (i = 0; i < size; i += PGSIZE)
    {
      if (find_spte ((uint8_t *)addr + i) != NULL)
        {
          f->eax = MAPID_ERROR;
          return;
        }
    }

  /* Insert mmap infomation. */
  minfo = (struct mmap_info *) malloc (sizeof (struct mmap_info));
  minfo->addr = addr;
  minfo->size = size;
  minfo->mapid = t->next_mapid++;

  list_push_back (&t->mmap_list, &minfo->elem_mmap);

  /* Allocate supplement pages. */
  of = 0;
  while (size > 0)
    {
      read_size = size > PGSIZE ? PGSIZE : size;
      new_file = file_reopen (file);
      spte = allocate_mmap (addr, NULL, new_file, read_size, of);
      spte->exist = false;
      size -= read_size;
      addr += read_size;
      of += read_size;
    }
  
  f->eax = minfo->mapid;
#endif
}

static void
sys_munmap (void *esp, struct intr_frame *f UNUSED)
{
#ifdef VM
  mapid_t mapid;
  struct list_elem *e;
  struct thread *t = thread_current ();
  struct page_table_entry *spte;
  struct mmap_info *minfo;
  void *addr = NULL;
  off_t size = 0, write_size = 0;
  
  MOV (esp, mapid_t*, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (mapid, mapid_t, esp);

  /* Find mmap. */
  for (e = list_begin (&t->mmap_list); e != list_end (&t->mmap_list); e = list_next (e))
    {
      minfo = list_entry (e, struct mmap_info, elem_mmap);
      if (minfo->mapid == mapid)
        {
          list_remove (e);
          size = minfo->size;
          addr = minfo->addr;
          free (minfo);
          break;
        }
    }
  /* Cannot find mmap. */
  if (e == list_end (&t->mmap_list))
    return;
  
  /* Write to file. */
  while (size > 0)
    {
      spte = find_spte (addr);
      delete_frame (spte);

      lock_acquire (&spte->in_process_lock);
      write_size = spte->read_size;
      file_close (spte->file);
      free (deallocate_page (addr)); /* No one will require it. */
      
      addr += write_size;
      size -= write_size;
    }
  
#endif
}

static void
sys_halt (void *esp UNUSED, struct intr_frame *f UNUSED)
{
  power_off ();
}

static void
sys_exit (void *esp, struct intr_frame *f UNUSED)
{
  int status;
  
  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (status, int, esp);
  
  kill_process (status);   
}

static void
sys_exec (void *esp, struct intr_frame *f)
{
  char *input, *cmdline;
  tid_t tid;
  
  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (input, char *, esp);
  if (!validate (input, false, NULL))
    kill_process (ERR);

  cmdline = malloc (MAX_PATH);
  if (cmdline != NULL)
    {
      if (check_string (input))
        {
          strlcpy (cmdline, input, MAX_PATH);
          tid = process_execute (cmdline);
        }
      free (cmdline);
    }
  else 
    tid = TID_ERROR;

  f->eax = tid;
}

static void
sys_wait (void *esp, struct intr_frame *f)
{
  tid_t tid;
  int exit_code;
  
  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (tid, tid_t, esp);
  
  exit_code = process_wait (tid); 
  
  f->eax = exit_code;
}

static void
sys_create (void *esp, struct intr_frame *f)
{
  char *input, *file_name;
  unsigned size;
  bool success;
  
  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
 
  GET (input, char *, esp);
  if (!validate (input, false, NULL))
    kill_process (ERR);

  MOV (esp, unsigned *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (size, unsigned, esp);

  file_name = malloc (MAX_PATH);

  if (file_name != NULL)
    {
      if (check_string (input))
        {
          strlcpy (file_name, input, MAX_PATH);
          success = filesys_create (file_name, size);
        } 
      free (file_name);
    }
  else
    success = false;
 
  f->eax = success;
}

static void
sys_remove (void *esp, struct intr_frame *f)
{
  char *input, *file_name;
  bool success;
  
  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (input, char *, esp);
  if (!validate (input, false, NULL))
    kill_process (ERR);

  file_name = malloc (MAX_PATH);
  if (file_name != NULL)
    {
      if (check_string (input))
        {
          strlcpy (file_name, input, MAX_PATH);
          success = filesys_remove (file_name);
        }  
      free (file_name);
    } 
  else
    success = false;
   
  f->eax = success;
}

static void
sys_open (void *esp, struct intr_frame *f)
{
  char *input, *file_name;
  struct file *file;
  int fd;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, char **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (input, char *, esp);
  if (!validate (input, false, NULL))
    kill_process (ERR);

  file_name = malloc (MAX_PATH);
  if (file_name != NULL)
    {
      if (check_string (input))
        {
          strlcpy (file_name, input, MAX_PATH);
          file = filesys_open (file_name);
        }
      
      free (file_name);
    }
  else
    file = NULL;
    
  if (file != NULL)
    {
      file_info = (struct file_info *) malloc (sizeof (struct file_info));
      if (file_info != NULL)
        {
          fd = curr->next_fd;
          curr->next_fd++;
          file_info->fd = fd;
          file_info->file = file;
          list_push_back (&curr->file_list, &file_info->elem_file);
        }
      else
        {
          fd = -1;
          file_close (file);
        }
    }
  else
    fd = -1;
  
  f->eax = fd;
}

static void
sys_filesize (void *esp, struct intr_frame *f)
{
  int fd;
  int size;
  struct file_info *file_info;
  struct list_elem *e;
  struct thread *curr = thread_current ();

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (fd, int, esp);

  for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
       e = list_next (e))
    {
      file_info = list_entry (e, struct file_info, elem_file);
      if (file_info->fd == fd)
        break;
    }
  
  if (e != list_end (&curr->file_list))
    { 
      size = file_length (file_info->file);
    }
  else
    size = -1;

  f->eax = size;
}

static void
sys_read (void *esp, struct intr_frame *f)
{
  int fd;
  char *buffer;
  unsigned size, real_size, temp, rval;
  char c, null = '\0';
  struct list_elem *e;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
 
  GET (fd, int, esp);

  MOV (esp, void **, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (buffer, char *, esp); 
  if (!validate (buffer, true, esp))
    kill_process (ERR);

#ifdef VM
  if (!is_writable (buffer))
    kill_process (ERR);
#endif

  MOV (esp, unsigned *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
 
  GET (size, unsigned, esp);

  if (fd == 0)
    {
      temp = 0;
      real_size = 0;
      while (temp < size)
        {
#ifdef VM
          if (temp > PGSIZE - pg_ofs (buffer))
            {
              unfix (buffer); 
              real_size += temp;
              size -= temp;
              temp = 0;
              if (!validate (buffer + temp, true, buffer + temp ) || !is_writable (buffer))
                kill_process (ERR);
              buffer += temp;
            }
#endif
          c = buffer[temp++] = input_getc ();
          if (c == null)
            break;
        }
      real_size += temp;
    }
  else
    {
      for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
           e = list_next (e)) 
        {
          file_info = list_entry (e, struct file_info, elem_file);
          if (file_info->fd == fd)
            break;
        }
      if (e == list_end (&curr->file_list))
        real_size = -1;
      else
        {
          real_size = 0;
          while (size > 0)
            {
              temp = size < (PGSIZE - pg_ofs (buffer)) ? size : PGSIZE - pg_ofs (buffer); 
              rval = file_read (file_info->file, buffer, temp);
              real_size += rval;
              size -= rval;
              if (rval < temp)
                break;
#ifdef VM
              unfix (buffer);
              if (!validate (buffer + rval, true, buffer + rval ) || !is_writable (buffer + rval))
                 kill_process (ERR);
#endif
              buffer += rval;
            }
        }
    }
 
  f->eax = real_size;
}

static void
sys_write (void *esp, struct intr_frame *f)
{
  int fd;
  void *buffer;
  unsigned size, real_size, temp;
  struct list_elem *e;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (fd, int, esp);

  MOV (esp, void **, 1);
  if (!validate (esp, false ,NULL))
    kill_process (ERR);
  
  GET (buffer, void *, esp);
  if (!validate (buffer, false, NULL))
    kill_process (ERR);

  MOV (esp, unsigned *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);
  
  GET (size, unsigned, esp);

  if (fd == 1)
    {
      real_size = size;
      while (size > DEFAULT_WRITE)
        {
          putbuf (buffer, DEFAULT_WRITE);
          size -= DEFAULT_WRITE;
          buffer += DEFAULT_WRITE;
#ifdef VM
          validate (buffer, false, NULL);
#endif
        }
       putbuf (buffer, size);
    }
  else
    {
      for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
           e = list_next (e)) 
        {
          file_info = list_entry (e, struct file_info, elem_file);
          if (file_info->fd == fd)
            break;
        }
      if (e == list_end (&curr->file_list) || file_is_dir (file_info->file))
        real_size = -1;
      else
        {
          real_size = 0;
	  while (real_size < size)
	    {
              temp = size - real_size > PGSIZE - pg_ofs (buffer) ? PGSIZE - pg_ofs (buffer) : size - real_size;
              temp = file_write (file_info->file, buffer, temp);
              if (temp == 0)
                break;
              else
                {
                  real_size += temp;
	          buffer += temp;
#ifdef VM
                  validate (buffer, false, NULL);
#endif
                }
	    }
        }
    }
 
  f->eax = real_size;
}

static void
sys_seek (void *esp, struct intr_frame *f UNUSED)
{
  int fd;
  struct file_info *file_info;
  unsigned position;
  struct list_elem *e;
  struct thread *curr = thread_current ();

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);
  
  MOV (esp, unsigned *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (position, unsigned, esp);

  for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
       e = list_next (e))
    {
      file_info = list_entry (e, struct file_info, elem_file);
      if (file_info->fd == fd)
        break;
    }
  
  if (e != list_end (&curr->file_list))
    { 
      file_seek (file_info->file, position); 
    }
}

static void
sys_tell (void *esp, struct intr_frame *f)
{
  int fd;
  struct file_info *file_info;
  struct thread *curr = thread_current ();
  struct list_elem *e;
  unsigned position;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);
  
  for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
       e = list_next (e))
    {
      file_info = list_entry (e, struct file_info, elem_file);
      if (file_info->fd == fd)
        break;
    }
  if (e != list_end (&curr->file_list))
    {
      position = file_tell (file_info->file);
    }
  else
   position = -1;

  f->eax = position;
}

static void
sys_close (void *esp, struct intr_frame *f UNUSED)
{
  int fd;
  struct file_info *file_info;
  struct thread *curr = thread_current ();
  struct list_elem *e;

  MOV (esp, int *, 1);
  if (!validate (esp, false, NULL))
    kill_process (ERR);

  GET (fd, int, esp);
  
  for (e = list_begin (&curr->file_list); e != list_end (&curr->file_list);
       e = list_next (e))
    {
      file_info = list_entry (e, struct file_info, elem_file);
      if (file_info->fd == fd)
        {
          list_remove (e);
          break;
        }
    }
  if (e != list_end (&curr->file_list))
    {
      file_close (file_info->file); 
      free (file_info);
    }
}
