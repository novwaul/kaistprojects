/*******************************/
/*File name : sgrep.c          */
/*For : assignment 2           */
/*Made by : 20160788 Inje Hwang*/
/*******************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for getopt */
#include "str.h"

#define FIND_STR        "-f"
#define REPLACE_STR     "-r"
#define DIFF_STR        "-d"

#define MAX_STR_LEN 1023

#define FALSE 0
#define TRUE  1

typedef enum {
  INVALID,
  FIND,
  REPLACE,
  DIFF
} CommandType;

/*---------------------------------------------------------------------*/
/*CloseFile()                                                          */
/*Check whether file closed well or not                                */
/*Parameter : file1, file2, filename1, filename2                       */
/*Return value : TRUE(valid), FALSE(invalid)                           */
/*About Stream : Give error message related to the fali of closing file*/
/*About Global variable : Use TRUE and FALSE                           */
/*---------------------------------------------------------------------*/


int
CloseFile(FILE *file1, FILE *file2, const char *filename1,
	  const char* filename2)
{
  int c, d;
  
  c = fclose(file1);
  d = fclose(file2);

  if((c == EOF) && (d == EOF)){
    fprintf(stderr, "Error: failed to close file %s and file %s\n",
	    filename1, filename2);
    return FALSE;
  }
  else if (c == EOF){
    fprintf(stderr, "Error: failed to close file %s", filename1);
    return FALSE;
  }
  else if (d == EOF){
    fprintf(stderr, "Error: failed to close file %s", filename2);
    return FALSE;
  }
  else
    return TRUE;
}
/*---------------------------------------------------------------------*/
/*PrintUsage()                                                         */
/*print out the usage of the Simple Grep Program                       */
/*Parameter : argv0                                                    */
/*Return value : None                                                  */
/*About Stream : Give usage message to stdout                          */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
void 
PrintUsage(const char* argv0) 
{
  const static char *fmt = 
    "Simple Grep (sgrep) Usage:\n"
    "%s [COMMAND] [OPTIONS]...\n"
    "\nCOMMNAD\n"
    "\tFind: -f [search-string]\n"
    "\tReplace: -r [string1] [string2]\n"
    "\tDiff: -d [file1] [file2]\n";

  printf(fmt, argv0);
}
/*---------------------------------------------------------------------*/
/*DoFind()                                                             */
/*Check whether substring is in file and print a line which contain it */
/*Parameter : pcSearch                                                 */
/*Return value : TRUE(valid), FALSE(invalid)                           */
/*About Stream : Get characters from stdin and store it in an array as */
/*               a string, give error messages to stderr and lines to  */
/*               stdout                                                */
/*-Error 1 : related to excess command line argument length            */
/*-Error 2 : related to excess input line length                       */
/*-lines : contains substring                                          */
/*About Global variable : Use TRUE, FALSE, MAX_STR_LEN                 */
/*---------------------------------------------------------------------*/
int
DoFind(const char *pcSearch)
{
  char buf[MAX_STR_LEN + 2];

  /*check cmd line argument length*/
  if (StrGetLength(pcSearch) > MAX_STR_LEN)
    {
      fprintf(stderr, "Error: argument is too long");
      return FALSE;
    }
  
  /*find subsrting*/
  while (fgets(buf, sizeof(buf), stdin)) {
    /* check input line length */
    if (StrGetLength((const char*)buf) > MAX_STR_LEN) {
      fprintf(stderr, "Error: input line is too long");
      return FALSE;
    }
    if (StrSearch((const char*)buf , pcSearch))
      printf("%s", buf);
    else
      continue;
  }
   
  return TRUE;
}  
/*---------------------------------------------------------------------*/
/*DoReplace()                                                          */
/*Replace a substring contained by several lines in a file into anothe */
/*substring and print the entire lines                                 */
/*Parameter : pcString1, pcString2                                     */
/*Return value : TRUE(valid), FALSE(invalid)                           */
/*About Stream : Get characters from stdin and store it in an array as */
/*               a string, give error messages to stderr and lines to  */
/*               stdout                                                */
/*-Error 1 : related to excess command line argument length            */
/*-Error 2 : related to 0 length substring                             */
/*-Error 3 : related to excess input line length                       */
/*-lines : all changed and unchanged lines which was in the file       */
/*About Global variable : Use TRUE, FALSE, MAX_STR_LEN                 */
/*---------------------------------------------------------------------*/
int
DoReplace(const char *pcString1, const char *pcString2)
{
  size_t len1, len2, i=0;
  char buf[MAX_STR_LEN + 2],
    replace[2*(MAX_STR_LEN + 2)], //to make replaced line
    *checkb, *checkr;

  /*check cmd line argument length*/
  if((len1 = StrGetLength(pcString1)) > MAX_STR_LEN){
    fprintf(stderr, "Error: argument is too long");
    return FALSE;
  }
  else if (len1 == 0){
    fprintf(stderr, "Error: Can't replace an empty substring");
    return FALSE;
  }
  if((len2 = StrGetLength(pcString2)) > MAX_STR_LEN){
    fprintf(stderr, "Error: argument is too long");
    return FALSE;
  }

  /*do replacing*/
  while(fgets(buf, sizeof(buf), stdin)){
    if(StrGetLength((const char*)buf) > MAX_STR_LEN){
      fprintf(stderr, "Error: input line is too long");
      return FALSE;
    }    
    StrCopy(replace, buf);
    checkb = buf;
    checkr = replace;
    while((checkb = StrSearch((const char*)checkb, pcString1))){
      checkr = StrSearch((const char*)checkr, pcString1);
      StrCopy(checkr, pcString2);
      StrCopy((checkr += len2), (checkb += len1));
    }
    while(i < (2*(MAX_STR_LEN + 2) -
	       StrGetLength((const char*)replace))){
      replace[StrGetLength(replace) + i] = '\0';
      i++;
    }
    printf("%s", replace);
    while(i < 2*(MAX_STR_LEN+2)){
      replace[i] = '\0';
      i++;
    }
  }

  return TRUE;
}
/*---------------------------------------------------------------------*/
/*DoDiff()                                                             */
/*Compare two files and print lines which is different from another    */
/*file                                                                 */
/*Parameter : file1, file2                                             */
/*Return value : TRUE(valid), FALSE(invalid)                           */
/*About Stream : Get characters from file stream and store it in an    */
/*               array as a string, give error messages to stderr and  */
/*               lines to stdout                                       */
/*-Error 1 : related to excess command line argument length            */
/*-Error 2 : related to failure of opening file                        */
/*-Error 3 : related to excess input line length                       */
/*-Error 4 : one file ended earlier than the other                     */
/*-lines : differernt lines to each other                              */
/*About Global variable : Use TRUE, FALSE, MAX_STR_LEN                 */
/*---------------------------------------------------------------------*/
int
DoDiff(const char *file1, const char *file2)
{
  FILE *filep1, *filep2;
  char buf1[MAX_STR_LEN + 2], buf2[MAX_STR_LEN + 2], *null1,
    *null2;
  size_t linenum = 0;

  /*check cmd line argument length*/
  if(StrGetLength(file1) > MAX_STR_LEN){
    fprintf(stderr, "Error: argument is too long");
    return FALSE;
  }
  if(StrGetLength(file2) > MAX_STR_LEN){
    fprintf(stderr, "Error: argument is too long");
    return FALSE;
  }

  /*open file*/
  filep1 = fopen(file1, "r");
  if (!filep1){
    fprintf(stderr, "Error: Failed to open file %s\n", file1);
    return FALSE;
  }
  filep2 = fopen(file2, "r");
  if (!filep2){
    fprintf(stderr, "Error: Failed to open file %s\n", file2);
    return FALSE;
  }

  /*do diff*/
  while ((null1 = fgets(buf1, sizeof(buf1), filep1)) &&
	 (null2 = fgets(buf2, sizeof(buf2), filep2))){
    ++linenum;
    if (StrGetLength((const char*)buf1) > MAX_STR_LEN){
      fprintf(stderr, "Error: input line %s is too long", file1);
      return FALSE;
    }
    if (StrGetLength((const char*)buf2) > MAX_STR_LEN){
      fprintf(stderr, "Error: input line %s is too long", file2);
      return FALSE;
    }
    if (StrCompare((const char*)buf1, (const char*)buf2) != 0){
      printf("%s@%lu:%s", file1, linenum, buf1);
      printf("%s@%lu:%s", file2, linenum, buf2);
    }
  }

  /*check whether one file ended earlier*/
  null2 = fgets(buf2, sizeof(buf2), filep2);
  if (!null1){
    if (null2 == NULL){
      if (CloseFile(filep1, filep2, file1, file2) == FALSE)
	return TRUE;
      return FALSE;
    }
    else{
      fprintf(stderr, "Error: %s ends early at line %lu",
	      file1, linenum);
      if (CloseFile(filep1, filep2, file1, file2) == FALSE)
	return FALSE;
      return FALSE;
    }
  }
  else{
    fprintf(stderr, "Error: %s ends early at line %lu",
	    file2, linenum);
    if (CloseFile(filep1, filep2, file1, file2) == FALSE)
      return FALSE;
    return FALSE;
  }

}
/*---------------------------------------------------------------------*/
/*CommandCheck()                                                       */
/*Check command type is valid and if valid, return command type        */
/*Parameter : argc, argv1                                              */
/*Return value : cmdtype(INVALID, FIND, REPLACE, DIFF), FALSE(invalid) */
/*about stream : None                                                  */
/*about Global variable : Use INVALID, FIND, REPLACE, DIFF, FALSE      */
/*---------------------------------------------------------------------*/ 
int
CommandCheck(const int argc, const char *argv1)
{
  int cmdtype = INVALID;
   
  /* check minimum number of argument */
  if (argc < 3)
    return cmdtype;
   
  /* check command type */ 
  if (StrCompare(argv1, FIND_STR) == 0) {
    if (argc != 3)
      return FALSE;    
    cmdtype = FIND;       
  }
  else if (StrCompare(argv1, REPLACE_STR) == 0) {
    if (argc != 4)
      return FALSE;
    cmdtype = REPLACE;
  }
  else if (StrCompare(argv1, DIFF_STR) == 0) {
    if (argc != 4)
      return FALSE;
    cmdtype = DIFF;
  }
   
  return cmdtype;
}
/*-------------------------------------------------------------------*/
/*main()                                                             */
/*Check retrun value of each function and terminate program          */
/*Paremeter : argc, argv                                             */
/*Return value : EXIT_FAILURE(invalid), EXIT_SUCCESS(valid)          */
/*About Stream : Get the number of command line arguments and        */
/*               themselves from stdin and give values to the        */
/*               operating system                                    */
/*About Global variable : Use FIND, REPLACE, DIFF                    */
/*-------------------------------------------------------------------*/
int 
main(const int argc, const char *argv[]) 
{
  int type, ret;
   
  /* Do argument check and parsing */
  if (!(type = CommandCheck(argc, argv[1]))) {
    fprintf(stderr, "Error: argument parsing error\n");
    PrintUsage(argv[0]);
    return (EXIT_FAILURE);
  }
   
  /* Do appropriate job */
  switch (type) {
  case FIND:
    ret = DoFind(argv[2]);
    break;
  case REPLACE:
    ret = DoReplace(argv[2], argv[3]);
    break;
  case DIFF:
    ret = DoDiff(argv[2], argv[3]);
    break;
  } 

  return (ret)? EXIT_SUCCESS : EXIT_FAILURE;
}
