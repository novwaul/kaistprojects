/**********************
 * EE209 Assignment 1 *
 **********************/

/* client.c */
/*20160788 Inje Hwang*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_NUM_NAME_CHARS 63
#define MAX_NUM_ID_CHARS 63
#define MAX_NUM_DIGITS   10

/*--------------------------------------------------------------------*/
/* Pre-defined error messages */
#define ERR_UNDEF_CMD   "ERROR: Undefined Command\n"
#define ERR_UNDEF_OPT   "ERROR: Undefined Option\n"
#define ERR_NEED_OPT    "ERROR: Need More Option\n"
#define ERR_SAME_OPT    "ERROR: Multiple Same Options\n"
#define ERR_AMBIG_ARG   "ERROR: Ambiguous Argument\n"
#define ERR_INVALID_ARG "ERROR: Invalid Option Argument\n"

/*--------------------------------------------------------------------*/

enum { FALSE = 0, TRUE };

typedef enum {
    C_EXIT,       /* exit */
    C_REG,        /* register */
    C_UNREG,      /* unregister */
    C_FIND,       /* find */
    C_FAIL,       /* failure */
    C_EOF         /* end of file */
} Command_T;

/*--------------------------------------------------------------------*/

typedef enum {
  NOOPT = 2,     /*when checkin no option*/
  CHECKOPT,      /*when checking option*/
  NOARG,         /*when checking no argument*/
  DUPLICATION,   /*when duplication occurs*/
  INVALIDARG,    /*when invalid argument occurs*/
  AMBIGARG,      /*when ambigouous argument occurs*/
  END            /*when program meets EOF or '\n' without any errors*/
} CASES;

typedef enum {
  GIVEFALSE =9,  /*prgram ends with error*/
  GIVETRUE       /*program ends correctly*/
} ENDRETURNVALUE;

enum {
  ON = 11,       /*indicate what function is running*/
  OFF            /*indicate what function is not running*/
};

typedef int
 checker,   /*the number of IDCHECK, NAMECHECK, 
              PURCHASECHECK funtion runs*/
 remember,  /*remember option among -n, -p, -i*/
 function   /*indicate two validate function*/
  ;

checker ID_CHECKER , PURCHASE_CHECKER , NAME_CHECKER ;
remember REMEMBER;
function  REG = OFF, UNREG_FIND = OFF;

/*--------------------------------------------------------------------*/
/* Check whether an exit command valid.                               */
/* Input: no argument                                                 */
/* Return value: TRUE(valid) or FALSE(invalid)                        */
/*--------------------------------------------------------------------*/
static int
ValidateExitCommand(void)
{
    int c;

    /* only blank command is allowed */
    while ((c = getchar()) != EOF && c != '\n' && isspace(c))
        ;
    if (c == EOF)
        exit(0);
    if (c != '\n' && c != EOF) {
        fprintf(stderr, "%s", ERR_UNDEF_OPT);

        /* eat the line */
        while ((c = getchar()) != EOF && c != '\n')
            ;
        if (c == EOF)
            exit(0);
        return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------------*/
/* -See input character and check conditions.(only for NOARG and      */
/*  END cases)                                                        */
/* -Print error messages in stderr by each case and return            */
/*  proper value.                                                     */
/* Input : character, EOF, and elements of CASES                      */
/* Return value : there are TRUE, FALSE and ENDRETURNVALUE            */
/* -TRUE(valid)                                                       */
/* -FALSE(invalid)                                                    */
/* -GIVETRUE(end correctly)                                           */
/* -GIVEFALSE(end with error)                                         */
/* Other Global variables used : ON, OFF                              */
/*--------------------------------------------------------------------*/
int ERRPrinter(int c, int CASENAME) 
{
  if (CASENAME == NOOPT){
    switch(c){
    case EOF: fprintf(stderr, "%s", ERR_NEED_OPT); exit(0);
    case'\n': fprintf(stderr, "%s", ERR_NEED_OPT); return FALSE;
    default: while ((c = getchar()) != EOF && (c != '\n'));//eat line
      fprintf(stderr, "%s", ERR_UNDEF_OPT);
      if(c == '\n') return FALSE;
      else exit(0);
    }
  }
  else if(CASENAME == CHECKOPT){
    switch(c){
    case EOF: fprintf(stderr, "%s", ERR_UNDEF_OPT); exit(0);
    case'\n': fprintf(stderr, "%s", ERR_UNDEF_OPT); return FALSE;
    default: while ((c = getchar()) != EOF && (c != '\n'));//eat line
      fprintf(stderr, "%s", ERR_UNDEF_OPT);
      if(c == '\n') return FALSE;
      else exit(0);
    }
  }
  else if(CASENAME == NOARG){
    switch(c){
    case EOF: fprintf(stderr, "%s", ERR_UNDEF_OPT); exit(0);
    case'\n': fprintf(stderr, "%s", ERR_UNDEF_OPT); return FALSE;
    default: return TRUE;
    }
  }
  else if (CASENAME == DUPLICATION){
    while ((c = getchar()) != EOF && c != '\n');
    fprintf(stderr, "%s", ERR_SAME_OPT);
    if (c == '\n') return FALSE;
    else exit(0);
  }
  else if (CASENAME == INVALIDARG){
    if(c != EOF && c != '\n')
      while((c = getchar()) != EOF && c != '\n');
    else;
    fprintf(stderr, "%s", ERR_INVALID_ARG);
    if (c == '\n') return FALSE;
    else exit(0);
  }
  else if (CASENAME == AMBIGARG){
    while((c = getchar())!= EOF && c != '\n');
    fprintf(stderr, "%s", ERR_AMBIG_ARG);
    if (c == '\n') return FALSE;
    else exit(0);
  }
  /*executed when ValidateRegisterCommand is running*/
  else if ((CASENAME == END) && (REG == ON)){
    switch(c){
    case EOF:
      if(NAME_CHECKER != 1 || ID_CHECKER != 1 ||
	 PURCHASE_CHECKER != 1){
	fprintf(stderr, "%s", ERR_NEED_OPT);
	exit(0);
      }
      else exit(0);
    case '\n':
      if(NAME_CHECKER != 1 || ID_CHECKER != 1 ||
	 PURCHASE_CHECKER != 1){
	fprintf(stderr, "%s", ERR_NEED_OPT);
	return GIVEFALSE;
      }
      else return GIVETRUE;
    default : assert(0); return 0; //cannot reach here
    }
  }
  /*executed when ValidateUnregisterOrFindCommand is running*/
  else if ((CASENAME == END) && (UNREG_FIND == ON)){
    switch(c){
    case EOF: exit(0);
    case '\n':return GIVETRUE;
    default : assert(0); return 0; //cannot reach here
    }
  }
  else{
    assert(0);
    return 0;
  }// cannot reach here
}

/*--------------------------------------------------------------------*/
/* -Check no argument case.                                           */
/* -Check ID argument.                                                */
/* Input : no argument                                                */
/* Return value : There are TRUE and ERRPrinter return value.         */
/* -TRUE(valid)                                                       */
/* -In case of invalid argument, return FALSE.                        */
/* -In case of no argument occurs, return FALSE.                      */
/* -If END case, check conditions and return GIVETRUE when it ends    */
/*  correctly or return GIVEFALSE for wrong end.                      */
/*--------------------------------------------------------------------*/
int IDCHECK(void)
{
  int len = 0;
  int c;

  /*eat space before meet argument*/
  while ((c = getchar()) != EOF && c != '\n' && isspace(c))
  ;

  /*check no argument case*/
  if (ERRPrinter(c, NOARG) == FALSE) return FALSE;
  else ungetc(c, stdin);

  /*check argument and length*/
  while ((c=getchar()) != '\n' && !isspace(c)){ //read argument
    if (c == EOF) return ERRPrinter(c, END);
    else if (isdigit(c) || isalpha(c) || c == '-' || c == '_' ||
	c== '.'){
      ++len;
      if(len > MAX_NUM_ID_CHARS){
	return ERRPrinter(c, INVALIDARG); //over the max length
     }
    }
    else return ERRPrinter(c, INVALIDARG);//invalid argument
  }
  if (c == ' ') return TRUE;
  else return ERRPrinter(c, END);
}

/*--------------------------------------------------------------------*/
/* -Check no argument case.                                           */
/* -Check PURCHASE argument.                                          */
/* Input : no argument                                                */
/* Return value : There are TRUE and ERRPrinter return value.         */
/* -TRUE(valid)                                                       */
/* -In case of invalid argument, return FALSE.                        */
/* -In case of no argument occurs, return FALSE.                      */
/* -If END case, check conditions and return GIVETRUE when it ends    */
/*  correctly or return GIVEFALSE for wrong end.                      */
/*--------------------------------------------------------------------*/
int PURCHASECHECK(void)
{
  int c;
  int len = 0;

  /*eat space before meet argument*/
   while ((c = getchar()) != EOF && c != '\n' && isspace(c))
    ;

  /*check no argument case*/  
   if (ERRPrinter(c, NOARG) == FALSE) return FALSE;
   else ungetc(c, stdin);

  /*check argument and length*/
  while ((c=getchar()) != '\n' && !isspace(c)){
    if (c == EOF) return ERRPrinter(c, END);
    if (isdigit(c)){
      if (c == '0' && len == 0){ // prevent first digit is '0'
	if(isdigit(c = getchar()))
	  return ERRPrinter(c, INVALIDARG);
	else ungetc(c, stdin); 
      }
      ++len;
      if(len > MAX_NUM_DIGITS)
	return ERRPrinter(c, INVALIDARG);//over max length
    }
    else
      return ERRPrinter(c, INVALIDARG);//invalid argument
  }
   if(c == ' ') return TRUE;
   else return ERRPrinter(c, END);
}

/*--------------------------------------------------------------------*/
/* -Check no argument case.                                           */
/* -Check NAME argument.                                              */
/* Input : no argument                                                */
/* Return value : There are TRUE and ERRPrinter return value.         */
/* -TRUE(valid)                                                       */
/* -In case of invalid argument, return FALSE.                        */
/* -In case of no argument occurs, return FALSE.                      */
/* -If END case, check conditions and return GIVETRUE when it ends    */
/*  correctly or return GIVEFALSE for wrong end.                      */
/*--------------------------------------------------------------------*/
int NAMECHECK(void)
{
  int len = 0;
  int c;
  /*eat space before meet argument*/
  while ((c = getchar()) != EOF && c != '\n' && isspace(c))
    ;
  /*check no argument case*/ 
  if (ERRPrinter(c, NOARG)==FALSE) return FALSE;
  else ;
    
  if (c  == '\''){ // check argument that starts with '''
    while((c = getchar()) != '\n' && c != '\'' ){
      if (c == EOF) return ERRPrinter(c, END);
      if (isalpha(c) || c == '-' ||  c == '.'|| c == '\\' ||
	  isspace(c)){
	if(c == '\\'){
	  c = getchar();
	  switch(c){
	  case '\n': return ERRPrinter(c, INVALIDARG);
	  case EOF:  return ERRPrinter(c, INVALIDARG);
	  case '\\': return ERRPrinter(c, INVALIDARG);
	  case '-': ++len; break;
	  case ' ': ++len; break;
	  case '.': ++len; break;
	  case '\'': ++len; break;
	  default: ++len; break;
	  }
	}
	else ++len;
	if (len > MAX_NUM_NAME_CHARS)// over max length
	  return ERRPrinter(c, INVALIDARG);
	else continue;
      }
      else return ERRPrinter(c, INVALIDARG);
    }
    if (c == '\''){//check argument ends correctly
      if(len < 1)
	return ERRPrinter(c, INVALIDARG);
      if((c = getchar()) == '\n' || c == EOF)
	return ERRPrinter(c, END);
      if(c == ' ')
	 return TRUE;
      else return ERRPrinter(c, INVALIDARG);
    }
    else return ERRPrinter(c, INVALIDARG);
  }
  
  else{ //check argument which does not start with '''
    ungetc(c, stdin);
    while((c = getchar()) != '\n' && !isspace(c)){
      if (c == EOF) return ERRPrinter(c, END);
      if (isalpha(c) || c == '-' || c == '.' || c == '\\'||
	  c == '\''){
	if(c == '\\'){
	  c = getchar();
	  switch(c){
	  case '\n': return ERRPrinter(c, END);
	  case EOF: return ERRPrinter(c, END);
	  case '\\': return ERRPrinter(c, INVALIDARG);
	  case ' ':  ++len; break;
	  case '-': ++len; break;
	  case '.': ++len; break;
	  case '\'': ++len; break;
	  default: ++len; break;
	  }
	}
	else ++len; 
	if (len > MAX_NUM_NAME_CHARS)// over max length
	  return ERRPrinter(c, INVALIDARG);
	else continue;
      }
      else
	return ERRPrinter(c, INVALIDARG);//invalid argument
    }
    if (c == ' ') return TRUE;
    else return ERRPrinter(c, END);
  }
}

/*--------------------------------------------------------------------*/
/* Check whether a reg(register) command valid.                       */
/* Input: no argument                                                 */
/* Return value: There are TRUE, FALSE and ERRPrinter return value.   */
/* -TRUE(valid)                                                       */ 
/* -FALSE(invalid)                                                    */
/* -In case of no option, return FALSE.                               */
/* -In case of undefined option, return FALSE.                        */
/* -In case of duplication, return FALSE.                             */
/* Global variables used : ON, OFF                                    */
/*--------------------------------------------------------------------*/
static int
ValidateRegisterCommand(void)
{
 int c;
 ID_CHECKER = PURCHASE_CHECKER = NAME_CHECKER = 0; 

 while(TRUE){

   REG = ON;
   
 /*eat spaces before meet -n, -i, -p */
   while ((c = getchar()) != EOF && c != '\n' && isspace(c));
      
 /*get out of this loop*/
   if ((c=='\n'||c == EOF)&&
       (NAME_CHECKER >= 1|| ID_CHECKER >=1 ||PURCHASE_CHECKER >= 1))
     break;
   else;
   
 /*check '-' or no argument*/
   if (c==EOF || c=='\n'|| c!='-')
     return ERRPrinter(c, NOOPT);
   else;

 /*check i, p and n*/
   REMEMBER = getchar();              
   if (REMEMBER!='i' && REMEMBER!='n'&& REMEMBER!='p')
     return ERRPrinter(REMEMBER, CHECKOPT);
   else;

 /*check a space exists*/
   c = getchar();                
   if (c==EOF || c=='\n'|| !isspace(c))
     return ERRPrinter(c, CHECKOPT);

  /*check argument and duplication*/
   switch (REMEMBER){                         
     case 'i': ++ID_CHECKER; 
       if (PURCHASE_CHECKER >= 2 || NAME_CHECKER >= 2
	   || ID_CHECKER >= 2) //check duplication
         return ERRPrinter(c, DUPLICATION);
       c = IDCHECK();
       REG = OFF;
       if (c == FALSE) return FALSE;//argument is invalid
       else if (c == TRUE) continue;//argument is valid
       else if (c == GIVETRUE) return TRUE;//ends correctly
       else if (c == GIVEFALSE) return FALSE;//ends noncorrectly
       else {
	 assert(0); // cannot reach here
	 return 0;
       }

   case 'p': ++PURCHASE_CHECKER; 
       if (PURCHASE_CHECKER >= 2 || NAME_CHECKER >= 2
	   || ID_CHECKER >= 2)
	 return ERRPrinter(c, DUPLICATION);
       c = PURCHASECHECK();
       REG = OFF;
       if (c == FALSE) return FALSE;
       else if (c == TRUE) continue;
       else if (c == GIVETRUE) return TRUE;
       else if (c == GIVEFALSE) return FALSE;
       else{
	 assert(0); //cannot reach here
	 return 0;
       }

   case 'n' : ++NAME_CHECKER; 
       if (PURCHASE_CHECKER >= 2 || NAME_CHECKER >= 2
	   || ID_CHECKER >= 2)
	 return ERRPrinter(c, DUPLICATION);
       c = NAMECHECK();
       REG = OFF;
       if (c == FALSE) return FALSE;
       else if (c == TRUE) continue;
       else if (c == GIVETRUE) return TRUE;
       else if (c == GIVEFALSE) return FALSE;
       else {
	 assert(0); //cannot reach here
	 return 0;
       }
   default: assert(0); return 0; //cannot reach here
   }
 }
 c = ERRPrinter(c, END);
 if (c == GIVETRUE) return TRUE;
 else if (c == GIVEFALSE) return FALSE;
 else{
   assert(0); // cannot reach here
   return 0;
 }
}


/*--------------------------------------------------------------------*/
/* Check whether an unreg(unregister) or a find(search) command valid.*/
/* If argument is ambiguous, it prints proper error message.          */
/* Input: no argument                                                 */
/* Return value: There are TRUE, FALSE and ERRPrinter return value.   */
/* -TRUE(valid)                                                       */ 
/* -FALSE(invalid)                                                    */
/* -In case of no option, return FALSE.                               */
/* -In case of undefined option, return FALSE.                        */
/* -In case of duplication, return FALSE.                             */
/* Global variables used : ON, OFF                                    */
/*--------------------------------------------------------------------*/
static int
ValidateUnregisterOrFindCommand(void)
{
  int c = 0;
  ID_CHECKER = NAME_CHECKER = 0;

  while(TRUE){

    UNREG_FIND = ON;
    
 /* eat space before meet -n, -p, -i*/
    while ((c = getchar()) != EOF && c != '\n' && isspace(c));
      
/*check '\n' to get out loop*/
   if ((c=='\n' || c == EOF)&&
       (NAME_CHECKER>=1 || ID_CHECKER>=1))
     break;
   else;
   
/*check '-' or no argument*/
   if (c==EOF || c=='\n'|| c!='-')
     return ERRPrinter(c, NOOPT);
   else;

 /*check i and n*/
   REMEMBER = getchar();         
   if (REMEMBER!='i' && REMEMBER!='n')
     return ERRPrinter(REMEMBER, CHECKOPT);
   else;

 /*check the space exists behind*/
   c = getchar();              
   if (c==EOF || c=='\n'|| !isspace(c))
     return ERRPrinter(c, CHECKOPT);
   else;

 /*check argument and duplication*/
   switch (REMEMBER){                         
     case 'i': ++ID_CHECKER;
       if (NAME_CHECKER == 1 && ID_CHECKER == 1)
	 return ERRPrinter(c, AMBIGARG);
       if (NAME_CHECKER >= 2 || ID_CHECKER >= 2) 
         return ERRPrinter(c, DUPLICATION);
       c = IDCHECK();
       UNREG_FIND = OFF;
       if (c  == FALSE) return FALSE;
       else if (c == TRUE) continue;
       else if (c == GIVEFALSE) return FALSE;
       else if (c == GIVETRUE) return TRUE;
       else{
	 assert(0);
	 return 0;
       }//cannot reach here
   default : ++NAME_CHECKER;
       if (NAME_CHECKER == 1 && ID_CHECKER == 1)
	 return ERRPrinter(c, AMBIGARG);
       if (NAME_CHECKER >= 2 || ID_CHECKER >= 2)
         return ERRPrinter(c, DUPLICATION);
       c = NAMECHECK();
       UNREG_FIND = OFF;
       if (c == FALSE) return FALSE;
       else if(c == TRUE) continue;
       else if (c == GIVEFALSE) return FALSE;
       else if (c == GIVETRUE) return TRUE;
       else{
	 assert(0);
	 return 0;
       }//cannot reach here
   }
  }// end of while loop
 /*exit this function */
  if(ERRPrinter(c, END) == GIVETRUE) return TRUE;
  else{
    assert(0);
    return 0;
  }
}


/*--------------------------------------------------------------------*/
/* Read the first word, and figure out and return the command type.   */
/* Input: no argument                                                 */
/* Return value: Command_T value                                      */
/*  - In case of an error, it eats the entire line and returns C_FAIL */
/*  - In case there's no more input (EOF), it returns C_EOF           */
/*--------------------------------------------------------------------*/
static Command_T
GetCommandType(void)
{
    Command_T type = C_FAIL;
    const char *cmds[] = {
        "exit",   /* exit */
        "reg",    /* reg */
        "unreg",  /* unreg */
        "find",   /* find */
    };
    int i, len;
    int c;
     
    /* eat space */
    while ((c = getchar()) != EOF && c != '\n' && isspace(c))
        ;

    switch (c) {
        case '\n':return C_FAIL;  /* no command */
        case EOF: return C_EOF;   /* no more input */
        case 'e': type = C_EXIT;  break;
        case 'r': type = C_REG;   break;
        case 'u': type = C_UNREG; break;
        case 'f': type = C_FIND;  break;
        default:
            fprintf(stderr, "%s", ERR_UNDEF_CMD);
            goto EatLineAndReturn;
    }
     
    /* see the rest of the command chars actually match */
    len = strlen(cmds[type]);
    for (i = 1; i < len; i++) {
        c = getchar();
        if (c == '\n' || c == EOF) {   /* line finished too early */
            fprintf(stderr, "%s", ERR_UNDEF_CMD);
            return (c == EOF) ? C_EOF : C_FAIL;
        }
        if (c != cmds[type][i]) {    /* wrong command */
            fprintf(stderr, "%s", ERR_UNDEF_CMD);
            goto EatLineAndReturn;
        }
    }

    /* check the following character of a command */
    c = getchar();
    if (c != '\n' && isspace(c)) {
        return type;
    } else if (c == '\n' || c == EOF) {
        /* only exit can be followed by '\n' */
        if (type != C_EXIT)
            fprintf(stderr, "%s", ERR_NEED_OPT);
        if (c == EOF)
            return C_EOF;
        else
            ungetc(c, stdin);
        if (type == C_EXIT)
            return type;
    } else {
        fprintf(stderr, "%s", ERR_UNDEF_CMD);
    }
    
EatLineAndReturn:
     while ((c = getchar()) != EOF && (c != '\n'))
        ;
    return (c == EOF) ? C_EOF : C_FAIL;
}
/*--------------------------------------------------------------------*/
/* -Run the program consistently.                                     */
/* -Check return value of validate functions.                         */
/* -Print '>' until the program is ended.                             */
/* Input : integer, character double pointer (do not use here)        */
/* Return value : 0 (for C_EXIT command and C_EOF only)               */
/* Global variables used : elements of Command_T                      */
/*--------------------------------------------------------------------*/
int
main(int argc, const char *argv[])
{
    Command_T command;
    int res;
    printf("======================================\n" \
           "  Customer Manager Program\n" \
           "======================================\n\n");

    /* start prompt */
    while (TRUE) {
        printf("\n> ");

        /* check command type */
        command = GetCommandType();

        /* command validation */
        switch (command) {
            case C_EOF:
                return 0;
            case C_FAIL:
                res = FALSE;
                break;
            case C_EXIT:
                res = ValidateExitCommand();
                break;
            case C_REG:
                res = ValidateRegisterCommand();
                break;
            case C_FIND:
                res = ValidateUnregisterOrFindCommand();
                break;
            case C_UNREG:
                res = ValidateUnregisterOrFindCommand();
                break;
            default:
                assert(0); /* can't reach here */
                break;
        }

        if (res == FALSE) {
            /* validation fail */
            continue;
        }

        /* command functionalities */
        switch (command) {
            case C_EXIT:
                exit(0);
                return 0;
            default:
                /* This will be expanded in assignment 3. */
                break;
        }
    }
    assert(0);  /* can't reach here */
    return 0;
}
