/*************************
 * Assignment 6          *
 * File name : ish.c     *
 * Author : InJe Hwang   *
 * Student ID : 20160788 *
 *************************/
#include "ish.h"

/*--------------------------------------------------------------------*/
/* myHandler                                                          */
/* Executed when SIGQUIT occurs. Wait 5 seconds until user make       */
/* SIGQUIT again. If user make SIGQUIT again in 5 seconds, exit the   */
/* program.                                                           */
/* > Parameter : int sig(never used)                                  */
/* > Return value : None                                              */
/* > Global varialbe : None                                           */
/*--------------------------------------------------------------------*/
static
void myHandler(int sig)
{
  unsigned int i;

    i = alarm(5); /* wait */

    if(i > 0)/* exit the program */
      exit(0);

    fprintf(stdout, "\nType Ctrl-\\ again within 5 seconds to exit\n");
    
}
/*--------------------------------------------------------------------*/
/* main                                                               */
/* Print prompt, take user input and store it in command array.       */
/* Send command or analyzed command(stored in command_storage) to     */
/* Lexical_Anlysis func, Sintaic_Analysis func, or Execution func.    */
/* when EXIT is returned, main func free all allocated memories and   */
/* terminate itself.                                                  */
/* > Parameter : int argc(never used), char* argv[]                   */
/* > Return value : 0 (only for exit command of user)                 */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  FILE *pFile;
  char *command = NULL, **command_storage = NULL, *path = NULL,
       *command_checker = NULL;
  int lexi_checker, open_checker = INVALID, close_checker = INVALID,
      synt_checker, exec_checker, i;
  uid_t user_id;
  struct passwd *user;
  sigset_t sSet;
  void(*pfRet_i)(int); void(*pfRet_q)(int); void(*pfRet_a)(int);
  
  sigemptyset(&sSet);
  sigaddset(&sSet, SIGALRM);
  sigaddset(&sSet, SIGINT);
  sigaddset(&sSet, SIGQUIT);
  if(sigprocmask(SIG_UNBLOCK, &sSet, NULL) != 0){
    assert(0);
  }
  
  pfRet_i = signal(SIGINT, SIG_IGN);
  pfRet_q = signal(SIGQUIT, myHandler);
  pfRet_a = signal(SIGALRM, SIG_IGN);
  if((pfRet_i == SIG_ERR) || (pfRet_a == SIG_ERR) ||
     (pfRet_q == SIG_ERR)){
    assert(0);
  }
  while( VALID ){
    /* print prompt */
    fprintf(stdout, "%% ");
    
    /* make command buffer */
    command = (char*)malloc(MAX_COMMAND_LEN);
    if( command == NULL ){
      fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
      continue;
    }
    /* store commands in command_storage */
    command_storage = (char **)calloc(sizeof(command),MAX_COMMAND_NUM);
    if( command_storage == NULL ){
      fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
      continue;
    }
    
    /* get command */
    if( open_checker == INVALID ){ /* check file has opened recently */
      user_id = getuid(); /* get user id */
      user = getpwuid(user_id); /* get user info */
      path = (char*)malloc(strlen("/home/") + strlen(user->pw_name) +
			   strlen("/.ishrc") );
      if( path == NULL ){
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	continue;
      }
      /* make path */
      strcpy(path, "/home/");
      strcat(path, user->pw_name);
      strcat(path, "/.ishrc");
      pFile = fopen(path, "r");/* open file */
      free(path);/* free path */
      open_checker = VALID; /* do not open this file again */
    }
    if((close_checker == INVALID) && (pFile != NULL)){
      /* when .ishrc is valid */
      command_checker = fgets(command, MAX_COMMAND_LEN, pFile);
      if( command_checker != NULL )
	fprintf(stdout, "%s", command );
      else{ /* already read all commands in .ishrc */
	if(close_checker == INVALID){ /* check recently closed file */
	  i = fclose(pFile);
	  if(i == -1){/* error in closing file */
	    assert(INVALID);
	  }
	  close_checker = VALID; /* do not close this file again */
	}
      }
    }
    if(((close_checker == INVALID) && (open_checker == INVALID))||
       ((close_checker == VALID) && (open_checker == VALID))){
      /* when .ishrc is invalid or read all commands from .ishrc */
      command_checker = fgets(command, MAX_COMMAND_LEN, stdin);
      assert(command_checker != NULL);
    }
    
    /* check commands is lexically valid */
    lexi_checker = Lexical_Analysis(command, command_storage, argv);
    if( lexi_checker == INVALID ){
      free(command_storage);
      free(command);
      continue;
    }

    /* check commands is syntatically valid */
    synt_checker = Syntatic_Analysis(command_storage, argv);
    if( synt_checker == INVALID ){
      Clean_Array(command_storage);
      free(command);
      free(command_storage);
      continue;
    }
    /* execute command */
    exec_checker = Execution(command_storage, argv);
    if( exec_checker == INVALID ){
      Clean_Array(command_storage);
      free(command);
      free(command_storage);
      continue;
    }
    else if( exec_checker == EXIT ){
      Clean_Array(command_storage);
      free(command);
      free(command_storage);
      exit(0);
    }

    /* delete all the buffers */
    free(command);
    Clean_Array(command_storage);
    free(command_storage);
  }
}
