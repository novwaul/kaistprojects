/*************************
 * Assignment 6          *
 * File name : imp.c     *
 * Author : InJe Hwang   *
 * Student ID : 20160788 *
 *************************/
#include "ish.h"

/*--------------------------------------------------------------------*/
/* Array_getLength                                                    */
/* Count how many tokens are in the command_storage.                  */
/* > Parameter : char** command_storage                               */
/* > Return value : i(the num of tokens)                              */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
int Array_getLength(char** command_storage)
{
  assert(command_storage != NULL);
  
  int i = 0;

  
  while(command_storage[i] != NULL)
    i++;
  return i;
}
/*--------------------------------------------------------------------*/
/* Clean_Array                                                        */
/* Free all the dynamically allocated memories stored in the          */
/* command_storage.                                                   */
/* > Parameter : char** command_storage                               */
/* > Return value : None                                              */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
void Clean_Array(char** command_storage)
{
  assert(command_storage != NULL);

  int command_num, i;


  /* get length of DynArray */
  command_num = Array_getLength(command_storage);
  /* clean DynArray */
  for(i = 0; i < command_num; i++) /* command_storage[0] is NULL */
    free(command_storage[i]);
}
/*--------------------------------------------------------------------*/
/* Lexical_Analysis                                                   */
/* Get command and check wehter user command is lexically correct.    */
/* Make tokens with command and store it in command_storage.          */
/* Return VALID for valid command, INVALID for invalid command        */
/* > Parameter : char* command, char** command_storage, char* argv[]  */
/* > Return value : VALID, INVALID                                    */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
int Lexical_Analysis(char* command, char** command_storage,
		     char* argv[])
{
  assert(command != NULL);
  assert(command_storage != NULL);
  assert(argv[0] != NULL);
  
  char *token1 = NULL, *qtoken = NULL, *temp = NULL, p;
  int command_len, i, prev, index = 0, len;


  /* check command length is valid */
  command_len = strlen(command);
  if(command_len >= MAX_COMMAND_LEN){
    fprintf(stderr, "%s: ERROR - exceed maximum command length",
	    argv[0]);
    return INVALID;
  }

  /* check there is at least one non-space character */
  p = command[0];
  while(isspace(p) != 0){
    i++;
    if( i >= command_len )
      return INVALID;
    p = command[i];
  }
  
  /* make tokens */
  for(i = 0; i < command_len; i++){
    p = command[i];
    if( isspace(p) != 0 ) /* ignore space */
      continue;
    
    /* check special character */
    if( p == '|' ){
      token1 = (char*)malloc(((len = strlen(PIPE)) + 1));
      if( token1 == NULL ){ /* not enough memory */
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	Clean_Array(command_storage);
	return INVALID;
      }
      strcpy(token1, PIPE);/* copy command */
      token1[len + 1] = '\0';
      /* add token in command_storage */
      command_storage[index] = token1;
      index++; /* increase index */
      token1 = NULL; /* reset token */
      continue;
    }
    
    else if ( p == '>' ){
      token1 = (char*)malloc(((len = strlen(REDOUT)) + 1));
      if ( token1 == NULL ){
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	Clean_Array(command_storage);
	return INVALID;
      }
      strcpy(token1, REDOUT);
      token1[len + 1] = '\0';
      command_storage[index] = token1;
      index++;
      token1 = NULL;
      continue;
    }
    
    else if ( p == '<' ){
      token1 = (char*)malloc(((len = strlen(REDIN)) + 1));
      if( token1 == NULL ){
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	Clean_Array(command_storage);
	return INVALID;
      }
      strcpy(token1, REDIN);
      token1[len+1] = '\0';
      command_storage[index] = token1;
      index++;
      token1 = NULL;
      continue;
    }

    else if ( p != '\"'){
      prev = i;
      p = command[++i];
      /* get command length before space or " */
      while ( (p != '\"') && (isspace(p) == 0)){
	if((p == '>') || (p == '<') || (p == '|'))
	  break;
	p = command[++i];
      }
      token1 = (char*) malloc(i - prev + 1);
      if(token1 == NULL){/* not enough memory */
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	Clean_Array(command_storage);
	return INVALID;
      }
      strncpy(token1, command + prev, i - prev);
      token1[i - prev] = '\0';
      if(qtoken != NULL){/* quoted token exist right before */
	temp = (char*)malloc(strlen(token1) + strlen(qtoken));
	/* merge tokens */
	strcpy(temp, qtoken);
	strcat(temp, token1);
	/* free token1 and qtoken */
	free(qtoken);
	free(token1);
	/* make token1 points to the merged token */
	token1 = temp;
	/* reset qtoken and temp */
	qtoken = temp = NULL;
      }
      if((isspace(p) != 0) || (p == '>') || (p == '<')
	 || (p == '|')){/* next is space or special char */
	command_storage[index] = token1;
	token1 = NULL;
	index++;
	if((p == '>') || (p == '<') || (p == '|'))
	  i--;
	continue;
      }
      else{/* next is " */
	i--; /* check token in next cycle */
	continue;
      }
    }
    
    else if ( p == '\"'){
      prev = i;
      p = command[++i];
      /* get command length between " and " */
      while( p != '\"'){
	if( (++i) >= command_len ){
	  fprintf(stderr, "%s: ERROR - Unmatched quote\n", argv[0]);
	  return INVALID;
	}
	p = command[i];
      }
      /* make qtoken */
      qtoken = (char*) malloc(i - prev);
      if(qtoken == NULL){ /* not enough memory */
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	Clean_Array(command_storage);
	return INVALID;
      }
      strncpy(qtoken, command + prev + 1,
	      i - prev - 1);/* between " and " */
      qtoken[i - prev - 1] = '\0';
      if(token1 != NULL){ /* it there is token right brfore */
	temp = malloc(strlen(token1) + strlen(qtoken));
	if(temp == NULL){/* not enough memory */
	  fprintf(stderr, "%s: ERROR - %s\n", argv[0],
		  strerror(errno));
	  Clean_Array(command_storage);
	  free(token1);
	  free(qtoken);
	  token1 = qtoken = NULL;
	  return INVALID;
	}
	/* merge tokens */
	strcpy(temp, token1);
	strcat(temp, qtoken);
	/* free token1, qtoken */
	free(token1);
	free(qtoken);
	/* make qtoken points merged token */
	qtoken = temp;
	/* reset */
	token1 = temp = NULL;
      }
      /* check whether space char exist right after " */
      if( isspace(p = command[++i]) != 0){
      /* add qtoken in command_storage */
	command_storage[index] = qtoken;
	index++; /* increase index */
	qtoken = NULL; /* reset qtoken */
	continue;
      }
      else{/* next is non-space char */
	i--;/* check token in next cycle */
	continue;
      }
    }
  }

  return VALID;
}
/*--------------------------------------------------------------------*/
/* Syntatic_Analysis                                                  */
/* Get token array(command_storage) and check wether user command     */
/* is syntatically correct. Return VALID for valid user command,      */ 
/* INVALID for invalid command.                                       */
/* > Parameter : char** command_storage, char*argv[]                  */
/* > Return value : INVALID, VALID                                    */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
int Syntatic_Analysis(char** command_storage, char* argv[])
{
  assert(command_storage != NULL);
  assert(command_storage[0] != NULL);
  assert(argv[0] != NULL);

  int command_num, i, prev_redin, prev_redout, prev_pipe;
  char* p;


  /* -1 means does not occur yet */
  prev_redin = prev_redout = prev_pipe = -1;

  /* get the number of command */
  command_num = Array_getLength(command_storage);

  /* check pipe and redirection */
  for(i = 0; i < command_num; i++){
    p = command_storage[i];
    if((strcmp(p, PIPE) == 0)|| (strcmp(p, REDIN) == 0)||
				   (strcmp(p, REDOUT) == 0)){
      /* check multiple redirection */
      if(strcmp(p,PIPE) == 0){
	if(prev_redout != -1){
	  fprintf(stderr, "%s: ERROR - Multiple redirection\n",
		  argv[0]);
	  return INVALID;
	}
      }
      else if(strcmp(p,REDIN) == 0){
	if(prev_pipe != -1){
	  fprintf(stderr, "%s: ERROR - Multiple redirection\n",
		  argv[0]);
	  return INVALID;
	}
	if(prev_redin != -1){
	  fprintf(stderr, "%s: ERROR - Multiple redirection\n",
		  argv[0]);
	  return INVALID;
	}
      }
      else{
	if(prev_redout != -1){
	  fprintf(stderr, "%s: ERROR - Multiple redirection\n",
		  argv[0]);
	  return INVALID;
	}
      }
      if((i == 0) || ((strcmp(p, PIPE) == 0) &&
		      (command_storage[i+1]==NULL))){
	/* missing command name */
	fprintf(stderr, "%s: ERROR - Missing command name\n", argv[0]);
	return INVALID;
	 }
      if(((p = command_storage[i + 1]) == NULL) ||
	 (strcmp(p, PIPE) == 0) || (strcmp(p, REDOUT) == 0) ||
	 (strcmp(p, REDIN) == 0)){/* missing destination */
	fprintf(stderr, "%s: ERROR - Pipe or redirection destination "
		"is not specified\n", argv[0]);
	return INVALID;
      }
      /* keep the location of special character */
      p = command_storage[i];
      if(strcmp(p,PIPE) == 0)
	prev_pipe = i;
      else if(strcmp(p,REDIN) == 0)
	prev_redin = i;
      else
	prev_redout = i;
    }
  }

  /* check the first command is built-in commands */
  p = command_storage[0];
  if(strcmp(p, "setenv") == 0){/* setenv */
    /* check parameter num */
    if((command_num == 1) || (command_num > 3)){
      fprintf(stderr, "%s: ERROR - setenv takes one or" 
	      " two parameters\n", argv[0]);
      return INVALID;
    }
    if((strcmp(command_storage[1], REDIN) == 0) ||
       (strcmp(command_storage[1], REDOUT) == 0)){
      fprintf(stderr, "%s: ERROR - setenv takes one or"
	      " two parameters\n", argv[0]);
      return INVALID;
    }
  }
  else if(strcmp(p, "unsetenv") == 0){/* unsetenv */
    /* check parameter num */
    if(command_num != 2){
      fprintf(stderr, "%s: ERROR - unsetenv takes one parameter\n",
	      argv[0]);
      return INVALID;
    }
  }
  else if(strcmp(p, "cd") == 0){/* cd */
    /* check parameter num */
    if(command_num > 2){
      fprintf(stderr, "%s: ERROR - cd takes zero or one parameter\n",
	      argv[0]);
      return INVALID;
    }
  }
  else if(strcmp(p, "exit") == 0){/* exit */
    if(command_num != 1){
      fprintf(stderr, "%s: ERROR - exit takes no parameter\n",
	      argv[0]);
      return INVALID;
    }
  }
  return VALID;
}
/*--------------------------------------------------------------------*/
/* Execution                                                          */
/* Get token array(command_storage) and execute programs or call      */
/* built-in functions. Return VALID when program or built-in func     */
/* well behaves, return INVALID for malfunction. But, exit command is */
/* not handled in Execution func. It returns EXIT when user give exit */
/* command.                                                           */
/* > Parameter : char** command_storage, char* argv[]                 */
/* > Return value : INVALID, VALID, EXIT                              */
/* > Global variable : None                                           */
/*--------------------------------------------------------------------*/
int Execution(char** command_storage, char* argv[])
{
  assert(command_storage != NULL);
  assert(argv[0] != NULL);

  char *p, *arg1, *arg2, **arg, *redin = NULL, *redout = NULL;
  int result, result2, pipe_num = 0, i, index = 0, STDIN,
    index_p, offset = 0, fd[2]; 
  struct passwd *user;
  uid_t user_id;
  pid_t pid, pid_checker;
  void(*pfRet_i)(int); void(*pfRet_q)(int);


  /* check the first command is built-in commands */
  p = command_storage[0];
  if(strcmp(p, "setenv") == 0){/* setenv */
    arg1 = command_storage[1];/* set arguments */
    arg2 = command_storage[2];
    if(arg2 == NULL)
      arg2 = "";
    result = setenv(arg1, arg2, VALID);
    if(result == -1){/* error occur */
      fprintf(stderr, "%s: ERROR - %s\n",argv[0], strerror(errno));
      return INVALID;
    }
    return VALID;
  }
  else if(strcmp(p, "unsetenv") == 0){/* unsetenv */
    arg1 = command_storage[1];/* set argument */
    result = unsetenv(arg1);
    if(result == -1){/* error occur */
      fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
      return INVALID;
    }
    return VALID;
  }
  else if(strcmp(p, "cd") == 0){/* cd */
    arg1 = command_storage[1];
    if(arg1 != NULL)
      result = chdir(arg1);
    else{
      user_id = getuid(); /* get user id */
      user = getpwuid(user_id); /* get user info */
      arg1 = (char*)malloc(strlen("/home/") + strlen(user->pw_name));
      if( arg1 == NULL ){/* not enough memory */
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	return INVALID;
      }
      /* make path */
      strcpy(arg1, "/home/");
      strcat(arg1, user->pw_name);
      /* change directory to home */
      result = chdir(arg1);
      /* free */
      free(arg1);
    }
    if(result == -1){/* error occur */
      fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
      return INVALID;
    }
    return VALID;
  }
  else if(strcmp(p, "exit") == 0){/* exit */
    return EXIT;
  }

  /* execute external programs */
  fflush(NULL);
  if((STDIN = dup(0)) == -1){// duplicate stdin
    assert(0);
  }
  pid = fork();
  if(pid == -1){// error occurs 
    assert(0);
  }
  
  else if(pid == 0){/* in child process */
    pfRet_i = signal(SIGINT, SIG_DFL);
    pfRet_q = signal(SIGQUIT, SIG_DFL);
    if((pfRet_i == SIG_ERR) || (pfRet_q == SIG_ERR)){
      assert(0);
    }
    /* get the number of pipes */ 
    for(i = 0; (p = command_storage[i]) != NULL; i++){
      if(strcmp(p, PIPE) == 0)
	pipe_num++;
    }
    for(i = 0; i < pipe_num ; i++ ){
      /* make pipe */
      if(pipe(fd) == -1){
	fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
	assert(0);
      }
      if(dup2(fd[0], 0) == -1){/* change read stream to pipe */
	assert(0);
      }
      if(close(fd[0]) == -1){
	assert(0);
	}
      if(pid == -1){// error occur
	assert(0);// quit process 
      }
      else if(pid == 0){// sub-child process goes in
	fflush(NULL);
	pid = fork();
	if(pid == 0){
	  if(dup2(fd[1], 1) == -1){
	     assert(0);
	  }
	  
	  if(close(fd[1]) == -1){
	    assert(0);
	    }
	}
	else{// sub-parent process goes in
	  if(close(fd[1]) == -1){
	    assert(0);
	  }
	  if(close(STDIN) == -1){
	    assert(0);
	  }
	  pid_checker = waitpid(pid, NULL, 0);
	  if(pid_checker == -1)// if error occurs in child 
	    assert(0);// quit process
	  break;
	}
      }
    }
    /* change the read stream of final child to STDIN,
       and close STDIN */
    if((pipe_num != 0) && (i == pipe_num)){
      if(dup2(STDIN, 0) == -1){
	assert(0);
      }
      if(close(STDIN) == -1){
	assert(0);
      }
    }
    /* make index to make execution command */
    index_p = pipe_num - i; /* to execute the first program
			       in the last child process */
    /* find right command by setting offset */
    for(p = command_storage[0], pipe_num = 0;
	(p != NULL) && (index_p != 0);
	p = command_storage[offset]){
      if(strcmp(p, PIPE) == 0)
	++pipe_num;
      if(pipe_num == index_p)
	break;
      ++offset;
    }
    if(pipe_num == 0)
      offset = -1; /* if index_p is 0, it starts from chararacter
		      ,but other cases starts from | */
    /* make execution command array */
    arg = (char**)calloc(1, Array_getLength(command_storage));
    if(arg == NULL){// error occurs 
      fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
      _exit(0);
    }
    /* make execution command */
    for(i = offset + 1; ((p = command_storage[i]) != NULL) &&
	  (strcmp(p,PIPE) != 0 ); i++){
      if(i == offset + 1){// get program name
	arg[index] = p;
	index++; //index is at first 0
	continue;
      }
      else if(strcmp(p, REDIN) == 0){// get redirection dest
	redin = command_storage[++i];
	continue;
      }
      else if(strcmp(p, REDOUT) == 0){
	redout = command_storage[++i];
	continue;
      }
      else{// get arguments
	arg[index] = p;
	index++;
      }
    }
    arg[index] = NULL; // the end of the arg must be NULL
    /* proceed redirection */
    if(redin != NULL){
      result = open(redin, O_RDONLY);//open in read-only version
      if(result == -1){// error occurs
	fprintf(stderr, "%s: ERROR - %s\n", argv[0],
		strerror(errno));
	free(arg);
	_exit(0);
      }
      result2 = dup2(result, 0);//redirection
      if(result2 == -1){// error occurs
	free(arg);
	assert(0);
      }
      if(close(result) == -1){// error in closing
	free(arg);
	assert(0);
      }
    }
    if(redout != NULL){
      result = open(redout, O_WRONLY| O_TRUNC, 0600);
      if(result == -1)// redout does not exist
	result = creat(redout, 0600);// create redout
      if(result == -1){// error occurs
	free(arg);
	assert(0);
      }
      result2 = dup2(result, 1);//redirection
      if(result2 == -1){// error occurs
	fprintf(stderr, "%s: ERROR - %s\n", argv[0],
		strerror(errno));
	free(arg);
        assert(0);
      }
      if(close(result) == -1){// error in closing 
	free(arg);
	assert(0);
      }
    }
    /* execute the program */
    execvp(arg[0], arg);// free arg automatically
    /* error occurs */
    fprintf(stderr, "%s: ERROR - %s\n", argv[0], strerror(errno));
    _exit(0);
  }/* child process ends */

  /* in parent process */
  else{
    assert(pid != 0);
    pid_checker = waitpid(pid, NULL, 0);
  if(pid_checker == -1)// if error occurs in child 
    return INVALID;
  else
    return VALID;
  }
}
