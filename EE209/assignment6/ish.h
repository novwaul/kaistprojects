/*************************
 * Assignment 6          *
 * File name : ish.h     *
 * Author : InJe Hwang   *
 * Student ID : 20160788 *
 **************************/
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_COMMAND_NUM 1024 /* actual maximum command num is 1022 
                                ,therefore cannot exceed 1024 */
#define MAX_COMMAND_LEN 1024 /* maximum input character is 1023 
                                ,therefore fit in 1024 array */

#define PIPE "PIPE"              /* to indicate pipe */
#define REDIN "REDIRECTION_IN"   /* to indicate redirection */
#define REDOUT "REDIRECTION_OUT"
#define EXIT - 1             /* to indicate that input is exit */
#define INVALID 0
#define VALID 1

/* Get command and check wehter user command is lexically correct.
   Make tokens with command and store it in command_storage.
   Return VALID for valid command, INVALID for invalid command   */
extern
int Lexical_Analysis(char* command, char** command_storage,
		     char* argv[]);

/* Get token array(command_storage) and check wether user command 
   is syntatically correct. Return VALID for valid user command, 
   INVALID for invalid command.                                   */
extern
int Syntatic_Analysis(char** command_storage, char* argv[]);

/* Get token array(command_storage) and execute programs or call 
   built-in functions. Return VALID when program or built-in func 
   well behaves, return INVALID for malfunction. But, exit command is 
   not handled in Execution func. It returns EXIT when user give exit 
   command.*/
extern
int Execution(char** command_storage, char* argv[]);

/* Count how many tokens are in the command_storage. */
extern
int Array_getLength(char** command_storage);

/* Free all the dynamically allocated memories stored in the 
   command_storage.                                          */
extern
void Clean_Array(char** command_storage);


