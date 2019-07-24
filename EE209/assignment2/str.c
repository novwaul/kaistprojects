/*-------------------------------*/
/*File Name : str.c              */
/*For : assignment2              */ 
/*Made by : 20160788 Inje Hwang  */
/*-------------------------------*/

#include <assert.h> /* to use assert() */
#include <stdio.h>
#include "str.h"


/* Your task is: 
   1. Rewrite the body of "Part 1" functions - remove the current
      body that simply calls the corresponding C standard library
      function.
   2. Write appropriate comment per each function
*/

/* Part 1 */
/*---------------------------------------------------------------------*/
/*Calculate length of a string                                         */
/*Parameter : pcSrc                                                    */
/*Return value : pcEnd - pcSrc(length of string)                       */
/*About Stream : None                                                  */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
size_t StrGetLength(const char* pcSrc)
{
  const char *pcEnd;
  assert(pcSrc); /* NULL address, 0, and FALSE are identical. */
  pcEnd = pcSrc;
	
  while (*pcEnd) /* null character and FALSE are identical. */
    pcEnd++;

  return (size_t)(pcEnd - pcSrc);
}

/*---------------------------------------------------------------------*/
/*Copy a string to an array                                            */
/*Parameter : pcDest, pcSrc                                            */
/*Return value : pcDest(copied string)                                 */
/*About Stream : None                                                  */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
char *StrCopy(char *pcDest, const char* pcSrc)
{
  size_t i;
  
  assert((pcDest != NULL) && (pcSrc != NULL));
  
  for (i = 0 ; pcSrc[i] != '\0' ; i++)
    pcDest[i] = pcSrc[i];
  pcDest[i] = '\0';
  
  return pcDest;
}

/*---------------------------------------------------------------------*/
/*First, compare characters of two stirings one by one using ASCII code*/
/*table and then compare string length                                 */
/*Parameter : pcS1, pcS2                                               */
/*Return Value : 1, 0, -1                                              */
/*-Value 1 : Left string is bigger(longer) than the right one          */
/*-Value 2 : Two strings are same                                      */
/*-Value -1 : Left string is smaller(shorter) than the right one       */
/*About Stream : None                                                  */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
int StrCompare(const char* pcS1, const char* pcS2)
{
  size_t i, len1, len2;
  
  assert((pcS1 != NULL) && (pcS2 != NULL));

  /*character comparison*/
  for(i = 0 ; (pcS1[i] != '\0') && (pcS2[i] != '\0') ; i++)
    {
      if (pcS1[i] == pcS2[i])
	continue;
      else
	{
	  if(pcS1[i] < pcS2[i])
	    return -1;
	  else
	    return 1;
	}
    }

  /*length comparison*/
  len1 = StrGetLength(pcS1);
  len2 = StrGetLength(pcS2);
  if (len1 < len2)
    return -1;
  else if (len1 > len2)
    return 1;
  else
    return 0;
}
/*---------------------------------------------------------------------*/
/*Check whether a substring is in another string or not                */
/*Parameter : pcHaystack, pcNeedle                                     */
/*Return value : NULL, pcHaystack, mark                                */
/*-NULL : Substring is not in another stirng                           */
/*-pcHaystack : Substring is ""                                        */
/*-mark : The location of substring that is in another string          */
/*About Stream : None                                                  */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
char *StrSearch(const char* pcHaystack, const char *pcNeedle)
{
  const char* mark; //get address of substring inside another string
  size_t i, j, lenH, lenN, len;

  assert((pcHaystack != NULL) && (pcNeedle != NULL));

  /*check first character*/
  if (*pcNeedle == '\0')
    return (char*) pcHaystack;

  /*search string*/
  for(i = 0 ; pcHaystack[i] != '\0' ; i++)
    {
      if(pcHaystack[i] == *pcNeedle)
	{
	  mark = pcHaystack + i;	  
	  lenH = StrGetLength(mark);
	  lenN = StrGetLength(pcNeedle);

	  if(lenH >= lenN)
	    len = lenN;
	  else
	    return NULL;

	  for(j = 0 ; j < len ; j++)
	    {
	      if(pcHaystack[i+j] == pcNeedle[j])
		{
		  if(j == (len - 1))
		    return (char*) mark;
		  else
		    continue;
		}
	      else
		break;
	    }
	}
    }
  return NULL;
}
/*---------------------------------------------------------------------*/
/*Concatenate strings                                                  */
/*Parameter : pcDest, pcSrc                                            */
/*Return value : pcDest(concatenated string)                           */
/*About Stream : None                                                  */
/*About Global variable : None                                         */
/*---------------------------------------------------------------------*/
char *StrConcat(char *pcDest, const char* pcSrc)
{
  size_t stpoint = 1;//changing start point

  assert((pcDest != NULL) && (pcSrc != NULL));

  /*check first character*/
  if (*pcSrc == '\0')
    {
      StrCopy(pcDest + stpoint , pcSrc);
      return (char*) pcDest;
    }

  /*do concatenation*/
  while(pcSrc[stpoint] != '\0')
    stpoint++;
  StrCopy(pcDest + stpoint, pcSrc);
  return (char*) pcDest;

}
