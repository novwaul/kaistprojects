#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"
#include "filesys/off_t.h"
#ifdef FILESYS
#include "filesys/directory.h"
#endif

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

enum process_status
  {
    PROCESS_RUN,
    PROCESS_END  
  };

typedef int mapid_t;
#define MAPID_ERROR ((mapid_t) -1);	/* Error value for mapid_t */

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */
#define NICE_MIN -20                    /* Loweset nice. */
#define NICE_DEFAULT 0                  /* Default nice. */
#define NICE_MAX 20                     /* Highest nice. */

#define MAX_STACK_CNT 1000		/* Maximum # of stack pages.*/
#define USER_BASE 0x08048000		/* Minimum user pointer value. */

/* Child process infomation. */
struct child_info
  {
    tid_t tid;                          /* Used as PID. */

    enum process_status status;         /* Process status. 
                                           Initially, process does not have any status. 
                                           After successfully loading a program, 
                                           process can have status. */

    int exit_code;                      /* Process exit code. */
    bool load;				/* Is process loaded successfully? */
    struct thread *child_thread;        /* Pointer to child thread. */
    struct list_elem elem_child;        /* List elem for child_list. */
  };

/* Opened file infomation. */
struct file_info
  {
    int fd;                             /* File descriptor.*/
    struct file *file;		        /* File. */
    struct list_elem elem_file;         /* List elem for file_list. */
  };

/* MMAP file infomation. */
struct mmap_info
  {
    mapid_t mapid;			/* MAPID. */
    off_t size;				/* Size of mmap. */
    void *addr;				/* Start address of mmap. */
    struct list_elem elem_mmap;		/* List elem for mmap_list. */
  };

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    struct list donator_list;           /* List of donator. */
    struct thread *thread_donated;      /* A donated thread by this thread. */
    
    int64_t wakeup_time;                /* Thread wake up time. */
    
    /* Owned by thread.c 
       and userprog/process.c */
    tid_t tid;                          /* Thread identifier. 
                                           Also same as process ID. */
    
    /* Owned by thread.c */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    int priority_donated;               /* The biggest donated priority. */
    int nice;                           /* Thread's nice value. */
    int recent_cpu;                     /* Thread's cpu usage time.
                                           It uses 17.14 fixed point representation. */

    struct list_elem elem_thread;       /* List element for tracking all threads.*/
    struct list_elem elem_donate;       /* List element for storing threads that donate 
                                           or will donate priority to this thread. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */
    /* Owned by timer.c. */
    struct list_elem elem_wakeup;       /* List element for wake up. */

    /* Owned by userprog/process.c. */
    bool user_thread;			/* Tell whether this thread is a user thread. */
    uint32_t *pagedir;                  /* Page directory. */
    struct file *file;                  /* File pointer of currnet running process. */
    int exit_code;                      /* Exit code of thread's process. */
    int next_fd;                        /* Next file descriptor #. */
    struct list child_list;             /* List for thread's child process infomation. */
    struct list file_list;              /* List for file infomation. */
    struct thread *parent;              /* Parent process.*/
    struct condition *finish;           /* A condition to synchronize waiting and exiting. */
    struct lock update_lock;            /* A lock for FINISH. */
    struct semaphore *wait;             /* A semaphore for executing and loading. */
    tid_t wait_tid;                     /* A tid that the current thread is waiting for. */

    /* Owned by vm/page.c */
    bool in_syscall;                    /* Indicate whether a process is handling system call, */
    mapid_t next_mapid;			/* Mmap ID. */
    struct list mmap_list;		/* List for mmap infomation. */
    struct hash *spt;			/* A pointer to supplemental page table. */
    uint32_t process_stack_page_count;  /* The number of a process stack pages. 
                                           Maximum is 100. */
    /* Owned by filesys, userprog directories.  */
#ifdef FILESYS
    uint32_t cur_dir_sector;		/* Current working directory sector. */
    struct dir *dir;
#endif
    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

list_less_func cmp_priority;

int thread_get_priority_any (const struct thread *t);

void thread_enroll_donator (struct thread *r, struct thread *d);
void thread_cancel_donator (struct thread *r, struct thread *d);
 
#endif /* threads/thread.h */
