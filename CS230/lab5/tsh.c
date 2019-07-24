/*
 * tsh - A tiny shell program with job control
 * 
 * InJe Hwang, 20160788
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

/* my Global variable */
volatile int finish_wait; /* if true, finsih waitfg function */
volatile int child_num = 0; /* the number of children */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Myfuncions */
void Setpgid(pid_t pid, pid_t pgid);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t * set, int option);
void Sigemptyset(sigset_t * set);
void Sigprocmask(int option, sigset_t * set, sigset_t * oset);
pid_t Fork(void);
void Execve(char* name, char** argv, char** environ);
void Kill(pid_t pid, int signal);
pid_t Waitpid(pid_t pid, int * status, int options);

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
	int bg, state;
	char* argv[MAXARGS];
	pid_t pid;
	sigset_t mask, prev, mask_all, prev_all;
	struct job_t * jobinfo;
	
	/* Signal setting */
	Sigfillset(&mask_all);
	Sigemptyset(&mask);
	Sigaddset(&mask, SIGCHLD);

	bg = parseline(cmdline, argv);
	if(!argv[0]) return; /* if user hits only enter key, return */
	state = bg ? BG : FG;
	if(!builtin_cmd(argv))/* interpret command */
	{
		Sigprocmask(SIG_BLOCK, &mask, &prev); /* block SIGCHLD */

		if((pid = Fork()) < 0)
		{
			Sigprocmask(SIG_SETMASK, &prev, NULL); /* unblock SIGCHILD */
			return;
		}
		else if(pid == 0)/* child */
		{
			Setpgid(0,0); /* set group ID */
			Sigprocmask(SIG_SETMASK, &prev, NULL); /* unblock SIGCHILD */
			Execve(argv[0], argv, environ);
		}
		else/* parent */
		{
			Sigprocmask(SIG_BLOCK,&mask_all,NULL);/* block singals */
			addjob(jobs, pid, state, cmdline);
			child_num += 1;
		}
		if(!bg)/* foreground job */ 
		{
			finish_wait = 0;
			Sigprocmask(SIG_SETMASK, &prev, NULL);/* unblock signals */
			waitfg(pid);/* check BG or FG */
		}
		else/* background job */
		{
			fprintf(stdout,"[%d] (%d) %s",pid2jid(pid),pid,cmdline);
			Sigprocmask(SIG_SETMASK, &prev, NULL);/* unblock signals */
		}
	}
	return;
}
/*---------------------------------Myfuncitons start-----------------------------------*/
/* Execute setpgid. If error occurs, print error msg and exit */
void Setpgid(pid_t pid, pid_t pgid)
{
	if(setpgid(pid, pgid) < 0)
	{
		unix_error("Setpgid error");
	}
	return;
}
/* Execute sigfillset. If error occurs, print error msg and exit */
void Sigfillset(sigset_t * set)
{
	if(sigfillset(set) < 0)
	{
		unix_error("Sigfillset error");
	}
	return;
}
/* Execute sigaddset. If error occurs, print error msg and exit */
void Sigaddset(sigset_t * set, int option)
{
	if(sigaddset(set, option) < 0)
	{
		unix_error("Sigaddset error");
	}
	return;
}
/* Execute sigemptyset. If error occurs, print error msg and exit */
void Sigemptyset(sigset_t * set)
{
	if(sigemptyset(set) < 0) 
	{
		unix_error("Sigemptyset error");	
	}
	return;
}
/* Execute sigprocmask. If error occurs, print error msg and exit */
void Sigprocmask(int option, sigset_t * set, sigset_t * oset)
{
	if(sigprocmask(option,set,oset) < 0)
	{	
		unix_error("Sigprocmask error");
	}
	return;
}
/* Return pid. Print error msg if error occurs */
pid_t Fork(void)
{
	pid_t pid;
	if((pid = fork()) < 0) fprintf(stderr,"Fork error: %s\n",strerror(errno));
	return pid;
}
/* Execute a program. If error occurs, print error msg and exit */
void Execve(char* name, char** argv, char** environ)
{
	if(execve(name, argv, environ) < 0)
	{
		fprintf(stderr,"%s: Command not found\n", name);
		exit(1);
	}
	else return;
}
/* Execute kill. If error occurs, print error msg and exit */
void Kill(pid_t pid, int signal)
{
	if(kill(pid, signal) < 0)
	{
		unix_error("Kill error");
	}
	return;
}
/* Execute waitpid. If error occurs, print error msg and exit */
pid_t Waitpid(pid_t pid, int * status, int options)
{
	pid_t return_pid;
	if((return_pid = waitpid(pid, status, options)) < 0)
		unix_error("Waitpid error");
	return return_pid;
}

/*----------------------------------Myfunctions end------------------------------*/
/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}
/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
	enum COMMAND {JOBS = 0, BACK, FORE, QUIT};
	enum COMMAND cmd;
	sigset_t mask_all, prev_all;

	Sigfillset(&mask_all);	

	/* find command */
	if(!strncmp(argv[0],"jobs",4)) cmd = JOBS;
	else if(!strncmp(argv[0],"bg",2)) cmd = BACK;
	else if(!strncmp(argv[0],"fg",2)) cmd = FORE;
	else if(!strncmp(argv[0],"quit",4)) cmd = QUIT;
	else return 0;     /* not a builtin command */
	
	switch(cmd){
	case JOBS: Sigprocmask(SIG_BLOCK,&mask_all,&prev_all);/* block signal */
		   listjobs(jobs);
		   Sigprocmask(SIG_SETMASK,&prev_all,NULL);/* unblock signal */
		   break;
	case QUIT: exit(0);
		   break;
	default:   do_bgfg(argv);/* BACK(BG) and FORE(FG) case */
		   break;
	}
	return 1;
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
	sigset_t mask_all, prev_all;
	struct job_t * jobinfo;
	int bg, state, pid, jid;

	Sigfillset(&mask_all);
	bg = !strncmp(argv[0],"bg",2);/* check bg */
	state = bg ? BG : FG;
	if(!argv[1])/* check second argumnet */
	{
		if(state == BG)
			fprintf(stderr,"bg command requires PID or %%jobid argument\n");/* arg num error */
		else
			fprintf(stderr,"fg command requires PID or %%jobid argument\n");/* arg num error */
		return;
	}
	Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);/* block signal */
	if(!strncmp(argv[1],"%%",1))/* JID argument */
	{
		if(!(jid = atoi(argv[1]+1)))
		{
			if(state == BG)
				fprintf(stderr, "bg: argument must be a PID or %%jobid\n");/* arg error */
			else
				fprintf(stderr, "fg: argument must be a PID or %%jobid\n");/* arg error */
			Sigprocmask(SIG_SETMASK, &prev_all, NULL); /* unblock signal */
			return;
		}
		else if(!(jobinfo = getjobjid(jobs,jid)))
		{
			fprintf(stderr,"%s: No such job\n",argv[1]);/* job does not exist */
			Sigprocmask(SIG_SETMASK, &prev_all, NULL); /* unblock signal */
			return;
		}
		
	}
	else/* PID argument */
	{
		if(!(pid = atoi(argv[1])))
		{
			if(state == BG)
				fprintf(stderr, "bg: argument must be a PID or %%jobid\n");/* arg error */
			else
				fprintf(stderr, "fg: argument must be a PID or %%jobid\n");/* arg error */
			Sigprocmask(SIG_SETMASK, &prev_all, NULL); /* unblock signal */
			return;
		}
		else if(!(jobinfo = getjobpid(jobs,pid)))
		{
			fprintf(stderr,"(%s): No such process\n",argv[1]);/* process does not exist */
			Sigprocmask(SIG_SETMASK, &prev_all, NULL); /* unblock signal */
			return;
		}
	}
	jobinfo->state = state; /* change state */
	finish_wait = bg ? 1 : 0; /* set finish_wait */
	pid = jobinfo->pid; /* get pid */
	Sigprocmask(SIG_SETMASK, &prev_all, NULL) ;/* unblock signal */
	Kill(-pid, SIGCONT);
	if(!bg) waitfg(pid); /* if FG, wait */
	else fprintf(stdout, "[%d] (%d) %s", jobinfo->jid, pid, jobinfo->cmdline); /* if BG, print msg */
	return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{	
	while(!finish_wait) sleep(1);
	return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
	pid_t pid;
	int i, status, olderrno = errno; /* store errno */
	struct job_t * jobinfo;
	sigset_t mask_all, prev_all;
	
	Sigfillset(&mask_all);
	Sigprocmask(SIG_BLOCK, &mask_all, &prev_all); /* block signal */			
	while(child_num > 0 && (pid = Waitpid(-1,&status,WUNTRACED | WCONTINUED | WNOHANG)) > 0) /* get pid */
	{
		jobinfo = getjobpid(jobs,pid);/* get job */
		if(WIFSTOPPED(status))/* child is stopped */
		{
			if(jobinfo->state == FG) finish_wait = 1;
			jobinfo->state = ST;/* change state to STOP */
			fprintf(stdout, "Job [%d] (%d) stopped by signal %d\n", 
				jobinfo->jid, jobinfo->pid, WSTOPSIG(status));
		}
		else if(WIFEXITED(status))/* child is terminated normally */
		{
			if(jobinfo->state == FG) finish_wait = 1;
			deletejob(jobs,pid);/* delete job */
			child_num -= 1;
		}
		else if(WIFSIGNALED(status)) /* child is terminated abnormally */
		{
			fprintf(stdout, "Job [%d] (%d) terminated by signal %d\n",
				jobinfo->jid, jobinfo->pid, WTERMSIG(status));
			if(jobinfo->state == FG) finish_wait = 1;
			deletejob(jobs,pid);
			child_num -= 1;	
		}
	}
	Sigprocmask(SIG_SETMASK,&prev_all,NULL); /* unblock signal */

	errno = olderrno; /* restore errno */
	return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
	int olderrno = errno;/* store errno */
	sigset_t mask_all, prev_all;
	pid_t pid;
	struct job_t *jobinfo;

	Sigfillset(&mask_all);
	
	Sigprocmask(SIG_BLOCK, &mask_all,&prev_all);/* block all signals */
	if(pid = fgpid(jobs)) Kill(-pid, SIGINT);
	Sigprocmask(SIG_SETMASK,&prev_all,NULL);/* unblock all signals */
	
	errno = olderrno;/* restore errno */
	return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
	int olderrno = errno;/* store errno */
	sigset_t mask_all, prev_all;
	pid_t pid;

	Sigfillset(&mask_all);
	
	Sigprocmask(SIG_BLOCK, &mask_all,&prev_all);/* block all signals */
	if(pid = fgpid(jobs)) Kill(-pid,SIGTSTP);
	Sigprocmask(SIG_SETMASK,&prev_all,NULL);/* unblock all signals */
	
	errno = olderrno; /* restore errno */
	return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



