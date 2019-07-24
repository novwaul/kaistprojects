#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "devices/timer.h"
#include "float.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif
#ifdef FILESYS
#include "filesys/filesys.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* Random value for basic thread
   Do not modify this value. */
#define THREAD_BASIC 0xd42df210  

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;

/* List of all processes. */
static struct list thread_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame 
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */
static int load_avg;            /* Estimates the average # of threads ready to run
                                   over the past minute. */

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

/* If false (default), use priority scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);

static int thread_calculate_priority (const struct thread *t);
static int thread_calculate_recent_cpu (const int coefficient, 
                                        const struct thread *t);
static int thread_calculate_load_avg (const int ready_threads);
static int thread_calculate_coefficient (void);
static void thread_update (void);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */

/* This is 2016 spring cs330 skeleton code */

void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&thread_list);
  load_avg = 0;

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
 
  /* Push it into thread_list. */
  list_push_back (&thread_list, &initial_thread->elem_thread);
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void) 
{
  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{
  struct thread *curr = thread_current ();

  /* Update statistics. */
  if (curr == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (curr->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  if (thread_mlfqs)
    thread_update ();

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();
}

/* Prints thread statistics. */
void
thread_print_stats (void) 
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;
  enum intr_level old_level;
  struct child_info *cinfo;

  ASSERT (function != NULL);

  old_level = intr_disable ();

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

  /* Enroll to child_list of parent. */
  if (t->parent != NULL)    
    {
      cinfo = (struct child_info *) malloc (sizeof (struct child_info));
      if (cinfo != NULL)
        {
          cinfo->tid = tid;
          cinfo->exit_code = 0;
          cinfo->load = false;
          cinfo->child_thread = t;
          list_push_back (&t->parent->child_list, 
                          &cinfo->elem_child);
        }
      else
        {
          palloc_free_page (t);
          return TID_ERROR;
        }
    }

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;

  /* Add to thread_list. */ 
  list_push_back (&thread_list, &t->elem_thread);

  thread_unblock (t);

  intr_set_level (old_level);

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  struct thread *curr = thread_current ();

  curr->status = THREAD_BLOCKED;

  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does preempt the running thread. */
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
   
  list_push_back (&ready_list, &t->elem);
  t->status = THREAD_READY;
  
  if (!intr_context ())
    thread_yield ();
  if (intr_context () && thread_current () == initial_thread)
    intr_yield_on_return ();

  intr_set_level (old_level);
}

/* Returns the name of the running thread. */
const char *
thread_name (void) 
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void) 
{
  struct thread *t = running_thread ();
  
  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void) 
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void) 
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  if (thread_current ()->user_thread)
    process_exit ();
#endif

  /* Just set our status to dying and schedule another process.
     We will be destroyed during the call to schedule_tail(). 
     Also, remove it from thread_list. */
  intr_disable ();
  list_remove (&thread_current ()->elem_thread);
  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void) 
{
  struct thread *curr = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();

  if (curr != idle_thread)
    list_push_back (&ready_list, &curr->elem);
  
  curr->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Sets the current thread's priority to NEW_PRIORITY. 
   If current thread does not have the highest priority, yeilds */
void
thread_set_priority (int priority) 
{
  struct thread *curr;
  enum intr_level old_level;

  /* When -mlfqs option is enabled, do nothing. */
  if (thread_mlfqs) return;
  
  old_level = intr_disable ();

  curr = thread_current ();
  curr->priority = priority;

  thread_yield();
  
  intr_set_level (old_level);
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_get_priority_any (thread_current ());
} 
  
/* Returns any thread's priority */
int
thread_get_priority_any (const struct thread *t)
{
  enum intr_level old_level = intr_disable ();
  int priority, priority_donated;
  
  priority = t->priority;
  priority_donated = t->priority_donated;

  intr_set_level (old_level);
  
  if (priority_donated > priority)
    return priority_donated;
  else
    return priority;
}

/* Enroll a new donator.
   If priority changes after enrollment, 
   re-donate the priority to the THREAD_DONATED of RECEIVER. */
void
thread_enroll_donator (struct thread *receiver, struct thread *donator)
{
  ASSERT (!thread_mlfqs);

  int priority_origin, priority_new;
  enum intr_level old_level;

  old_level = intr_disable ();

  priority_origin = thread_get_priority_any (receiver);
  priority_new = thread_get_priority_any (donator);

  list_push_back (&receiver->donator_list, &donator->elem_donate);

  donator->thread_donated = receiver;

  /* Check whether priority changes. */
  if (priority_new > priority_origin)
    {
      receiver->priority_donated = priority_new;

      if (receiver->thread_donated != NULL)
        {
          /* Redonate priority to THREAD_DONATED of RECEIVER. */
          thread_cancel_donator (receiver->thread_donated,
                                 receiver);
          thread_enroll_donator (receiver->thread_donated,
                                 receiver);
        }
    }
  
  intr_set_level (old_level);
}

/* Cancel Enrollment. 
   Also, check whether priority changes or not. 
   If changes, store the new priority. */
void
thread_cancel_donator (struct thread *receiver, struct thread *donator)
{
  ASSERT (!thread_mlfqs);
  int priority_re, priority_do, priority_new;
  enum intr_level old_level;
  struct list_elem *e;
  struct thread *t;

  old_level = intr_disable ();

  priority_re = thread_get_priority_any (receiver);
  priority_do = thread_get_priority_any (donator);

  list_remove (&donator->elem_donate);

  /* Check whether priority changes. */
  if (priority_re == priority_do
      && receiver->priority_donated > receiver->priority)
    {
      if (list_empty (&receiver->donator_list))
        receiver->priority_donated = 0;
      else
        {
          e = list_max (&receiver->donator_list, cmp_priority, NULL);
          t = list_entry (e, struct thread, elem_donate);
          priority_new = thread_get_priority_any (t);
          receiver->priority_donated = priority_new;
        }
    }  
  
  intr_set_level (old_level);
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice) 
{
  struct thread *curr;
  int old_priority;
  enum intr_level old_level;
 
  old_level = intr_disable ();
 
  curr = thread_current ();
  old_priority = curr->priority;
  
  curr->nice = nice;

  /* Calculate priority. */  
  curr->priority = thread_calculate_priority (curr);
  
  if (list_entry (
        list_max (&ready_list, cmp_priority, NULL), struct thread, elem)->priority 
      > curr->priority)
    thread_yield ();

  intr_set_level (old_level);
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{
  return  thread_current ()->nice;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void) 
{
  return roundfloat (mulfloatint (load_avg, 100));
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void) 
{
  int recent_cpu = thread_current ()->recent_cpu;
  return roundfloat (mulfloatint (recent_cpu, 100));
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED) 
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;) 
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux) 
{
  ASSERT (function != NULL);
                    
  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void) 
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Since `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;
  t->nice = NICE_DEFAULT;

  /* Initial thread has as 0 recent_cpu.
     Other threads except idle_thread has parent's recent_cpu.
     idle_thread has PRI_MIN priority. */
  if (thread_mlfqs)
    {
      if (t == initial_thread || strcmp (t->name, "idle") == 0)
        t->recent_cpu = 0;
      else
        t->recent_cpu = thread_current ()->recent_cpu;
      if (strcmp (t->name, "idle") != 0)
        priority = thread_calculate_priority (t);
    }
  t->priority = priority;
  t->priority_donated = 0;
  t->thread_donated = NULL;

  if (t == initial_thread || strcmp (t->name, "idle") == 0)
    {
      t->parent = NULL;
#ifdef FILESYS
      t->cur_dir_sector = ROOT_DIR_SECTOR;
      t->dir = NULL;
#endif
    }
  else
    {
      t->parent = thread_current ();
#ifdef FILESYS
      t->cur_dir_sector = t->parent->cur_dir_sector;
      t->dir = NULL;
#endif
    }

  t->user_thread = false;
  t->exit_code = 0;  /* 0 for normal, -1 for error. */
  t->next_fd = 2;    /* 0 for stdin, 1 for stdout. */                
  t->wait_tid = 0;   /* 0 for non-user thread. */
  t->wait = NULL;
  t->finish = NULL;
 
  list_init (&t->donator_list);
  list_init (&t->child_list);
  list_init (&t->file_list);
  list_init (&t->mmap_list);
  lock_init (&t->update_lock);

  t->in_syscall = false;
  t->spt = NULL;
  t->process_stack_page_count = 0;
  t->next_mapid = 0;
 
  t->magic = THREAD_MAGIC;
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size) 
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void) 
{
  struct list_elem *e;
  if (list_empty (&ready_list))
    return idle_thread;
  else
   {
     e = list_max (&ready_list, cmp_priority, NULL);
     list_remove (e);
     return list_entry (e, struct thread, elem); 
   }
}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
schedule_tail (struct thread *prev) 
{
  struct thread *curr = running_thread ();
  
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  curr->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
    {
      ASSERT (prev != curr);
      palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.
   
   It's not safe to call printf() until schedule_tail() has
   completed. */
static void
schedule (void) 
{
  struct thread *curr = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (curr->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (curr != next)
    prev = switch_threads (curr, next);
  schedule_tail (prev); 
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void) 
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/* Updates load_avg, thread's recent_cpu and priority. */
static void
thread_update (void)
{
  struct thread *t, *curr = thread_current ();
  struct list_elem *e;
  int ready_threads, coefficient, ticks = timer_ticks ();

  /* Increase recent_cpu of the current thread. */
  if (strcmp (curr->name, "idle") != 0)
      curr->recent_cpu += F;

  /* Upadte load_avg and for every 1 second. */
  if (ticks % TIMER_FREQ == 0)
    {
      ready_threads = list_size (&ready_list);
      if (strcmp (curr->name, "idle") != 0)
        ready_threads++;
      load_avg = thread_calculate_load_avg (ready_threads);

      coefficient = thread_calculate_coefficient ();
      for (e = list_begin (&thread_list); e != list_end (&thread_list); e = list_next (e))
        {
          t = list_entry (e, struct thread, elem_thread);
          if (strcmp (t->name , "idle") != 0)
            t->recent_cpu = thread_calculate_recent_cpu (coefficient, t);
        }
    }

  /* Update priority for every 4 ticks. */
  if (ticks % TIME_SLICE == 0)
    {
      for (e = list_begin (&thread_list); e != list_end (&thread_list); e = list_next (e))
        {
          t = list_entry (e, struct thread, elem_thread);
          if (strcmp (t-> name, "idle") != 0)
            t->priority = thread_calculate_priority (t);
        }
    }
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);

/* Compare whether priority of NEW is smaller than ORIGIN or not. */
bool
cmp_priority (const struct list_elem *new, 
              const struct list_elem *origin,
              void *aux UNUSED)
{
  struct thread *thread_o, *thread_n;
  int priority_o, priority_n;

  thread_o = list_entry (origin, struct thread, elem);
  thread_n = list_entry (new, struct thread, elem);

  priority_o = 
    (thread_o->priority_donated > thread_o->priority) 
      ? thread_o->priority_donated : thread_o->priority;

  priority_n = 
    (thread_n->priority_donated > thread_n->priority)
      ? thread_n->priority_donated : thread_n->priority;
  
  return (priority_n < priority_o);
}

/* Calculation functions are used when -mlfqs option is valid. */
/* Calculate recent_cpu. */
static int
thread_calculate_recent_cpu (const int coefficient, const struct thread *t)
{
  ASSERT (thread_mlfqs);

  return addfloatint (mulfloat (coefficient, t->recent_cpu), t->nice);
}

/* Calculate priority. */
static int
thread_calculate_priority (const struct thread *t)
{
  ASSERT (thread_mlfqs);
  int temp;
 
  temp = roundfloat (subfloatint (addfloatint (-divfloatint (t->recent_cpu, 4), PRI_MAX),  2 * t->nice));

  if (temp > PRI_MAX)
    return PRI_MAX;
  else if (temp < PRI_MIN)
    return PRI_MIN;
  else
    return temp;
}

/* Calculate coefficient that will be used in recent_cpu calculation. */
static int
thread_calculate_coefficient (void)
{
  ASSERT (thread_mlfqs);
  
  return divfloat (mulfloatint (load_avg, 2), addfloatint (mulfloatint (load_avg, 2), 1));
  
}

/* Calculate load_avg. */
static int
thread_calculate_load_avg (const int ready_threads)
{
  ASSERT (thread_mlfqs);

  return addfloat (divfloatint (mulfloatint (load_avg, 59), 60) , divfloatint (tofloat(ready_threads), 60));
}

