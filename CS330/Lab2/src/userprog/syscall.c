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
#include "userprog/ctrlsp.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

#define DEFAULT_WRITE  128

static void syscall_handler (struct intr_frame *);
static void kill_process (int exit_code);
static bool validate (void *p);
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

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  void *esp = f->esp;
  int idx;

  if (!validate (esp))
    kill_process (ERR);
  else
    {
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
        default: break;
        };
      return;
    }
}

/* Validate user pointer. */
static bool
validate (void *p)
{
  if (p == NULL)
    return false;
  else if (!is_user_vaddr (p))
    return false;
  else if (!pagedir_get_page (thread_current ()->pagedir, p))
    return false;
  else 
    return true;
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
sys_halt (void *esp UNUSED, struct intr_frame *f UNUSED)
{
  power_off ();
}

static void
sys_exit (void *esp, struct intr_frame *f UNUSED)
{
  int status;
  
  MOV (esp, int *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (status, int, esp);
  
  kill_process (status);   
}

static void
sys_exec (void *esp, struct intr_frame *f)
{
  char *cmdline;
  tid_t tid;
  
  MOV (esp, char **, 1);
  if (!validate (esp))
    kill_process (ERR);

  GET (cmdline, char *, esp);
  if (!validate (cmdline))
    kill_process (ERR);

  tid = process_execute (cmdline);

  f->eax = tid;
}

static void
sys_wait (void *esp, struct intr_frame *f)
{
  tid_t tid;
  int exit_code;
  
  MOV (esp, int *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (tid, tid_t, esp);
  
  exit_code = process_wait (tid); 
  
  f->eax = exit_code;
}

static void
sys_create (void *esp, struct intr_frame *f)
{
  char *file_name;
  unsigned size;
  bool success;

  MOV (esp, char **, 1);
  if (!validate (esp))
    kill_process (ERR);
 
  GET (file_name, char *, esp);
  if (!validate (file_name))
    kill_process (ERR);

  MOV (esp, unsigned *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (size, unsigned, esp);

  if (!(strlen(file_name) > 14))
    {
      lock_acquire (&filesys_lock);
      success = filesys_create (file_name, size);
      lock_release (&filesys_lock);
    }
  else 
    success = false;

  f->eax = success;
}

static void
sys_remove (void *esp, struct intr_frame *f)
{
  char *file_name;
  bool success;
  
  MOV (esp, char **, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (file_name, char *, esp);
  if (!validate (file_name))
    kill_process (ERR);

  if (!(strlen (file_name) > 14))
    {
      lock_acquire (&filesys_lock);
      success = filesys_remove (file_name);
      lock_release (&filesys_lock);
    }
  else
    success = false;

  f->eax = success;
}

static void
sys_open (void *esp, struct intr_frame *f)
{
  char *file_name;
  struct file *file;
  int fd;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, char **, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (file_name, char *, esp);
  if (!validate (file_name))
    kill_process (ERR);

  if (!(strlen (file_name) > 14))
    {
      lock_acquire (&filesys_lock);
      file = filesys_open (file_name);
      lock_release (&filesys_lock);
    }
  else
    file = NULL;

  if (file != NULL)
    {
      fd = curr->next_fd;
      curr->next_fd++;

      file_info = (struct file_info *) malloc (sizeof (struct file_info));
      if (file_info != NULL)
        {
          file_info->fd = fd;
          file_info->file = file;
          list_push_back (&curr->file_list, &file_info->elem_file);
        }
      else
        {
          fd = -1;
          lock_acquire (&filesys_lock);
          file_close (file);
          lock_release (&filesys_lock);
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
  if (!validate (esp))
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
      lock_acquire (&filesys_lock);
      size = file_length (file_info->file);
      lock_release (&filesys_lock);
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
  unsigned size, real_size;
  char c, null = '\0';
  struct list_elem *e;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, int *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (fd, int, esp);

  MOV (esp, void **, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (buffer, char *, esp);
  if (!validate (buffer))
    kill_process (ERR);

  MOV (esp, unsigned *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (size, unsigned, esp);

  if (fd == 0)
    {
      real_size = 0;
      while (real_size < size)
        {
          c = buffer[real_size] = input_getc ();
          real_size++;
          if (c == null)
            break;
        }
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
          lock_acquire (&filesys_lock);
          real_size = file_read (file_info->file, buffer, size);
          lock_release (&filesys_lock);
        }
    }
 
  f->eax = real_size;
}

static void
sys_write (void *esp, struct intr_frame *f)
{
  int fd;
  void *buffer;
  unsigned size, real_size;
  struct list_elem *e;
  struct thread *curr = thread_current ();
  struct file_info *file_info;

  MOV (esp, int *, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (fd, int, esp);

  MOV (esp, void **, 1);
  if (!validate (esp))
    kill_process (ERR);
  
  GET (buffer, void *, esp);
  if (!validate (buffer))
    kill_process (ERR);

  MOV (esp, unsigned *, 1);
  if (!validate (esp))
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
      if (e == list_end (&curr->file_list))
        real_size = -1;
      else
        {
          lock_acquire (&filesys_lock);
          real_size = file_write (file_info->file, buffer, size);
          lock_release (&filesys_lock);
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
  if (!validate (esp))
    kill_process (ERR);

  GET (fd, int, esp);
  
  MOV (esp, unsigned *, 1);
  if (!validate (esp))
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
      lock_acquire (&filesys_lock);
      file_seek (file_info->file, position);
      lock_release (&filesys_lock); 
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
  if (!validate (esp))
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
      lock_acquire (&filesys_lock);
      position = file_tell (file_info->file);
      lock_release (&filesys_lock);
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
  if (!validate (esp))
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
      lock_acquire (&filesys_lock);
      file_close (file_info->file); 
      lock_release (&filesys_lock);
      free (file_info);
    }
}
