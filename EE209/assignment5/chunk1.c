/*--------------------------------------------------------------------*/
/* chunk1.c                                                           */
/* Assignment 5                                                       */
/* Author: In Je Hwnag                                                */
/* Stuendt ID : 20160788                                              */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "chunkbase.h"

enum {
   MEMALLOC_MIN = 1024,
};

struct Chunk {
   Chunk_T next;     /* Pointer to the next chunk in the free chunk list */
   size_t units;     /* Capacity of a chunk (chunk units) */
};

/* gFreeHead, gFreeTail: point to first/last chunk in free list */
static Chunk_T gFreeHead = NULL, gFreeTail = NULL;

/* gHeapStart, gHeapEnd: start and end of the heap area.
 * gHeapEnd will move if you increase the heap */
static void *gHeapStart = NULL, *gHeapEnd = NULL;

/*--------------------------------------------------------------------*/
/* GetFooter                                                          */
/* Calculate the address of footer for doubly linked list and         */
/* return the address.                                                */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) footer                                   */
/*--------------------------------------------------------------------*/
static Chunk_T
GetFooter(Chunk_T c)
{
  Chunk_T footer;

  footer = c + c->units + 1;

  return footer;
}
/*--------------------------------------------------------------------*/
/* ChunkInitDS                                                        */
/* Initialize data structures and global variables for chunk          */
/* management.                                                        */
/* -Parameter : None                                                  */
/* -Return value : None                                               */
/* -Global variable : gHeapStart, gHeapEnd (initialize them)          */
/*--------------------------------------------------------------------*/
void
ChunkInitDS(void)
{
   /* Initialize gHeapStart if it is not initialized yet */
   gHeapStart = gHeapEnd = sbrk(0);
   if (gHeapStart == (void *)-1) {
      fprintf(stderr, "sbrk(0) failed\n");
      exit(-1);
   }
}
/*--------------------------------------------------------------------*/
/* ChunkGetStatus                                                     */
/* Returns a chunk's status which shows whether the chunk is in use or*/ 
/* free. Return value is either CHUNK_IN_USE or CHUNK_FREE.           */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (int) CHUNK_IN_USE, (int) CHUNK_FREE               */
/*--------------------------------------------------------------------*/
int
ChunkGetStatus(Chunk_T c)
{
   /* If a chunk is not in the free chunk list, then the next pointer
    * must be NULL. We can figure out whether a chunk is in use or
    * free by looking at the next chunk pointer. */
   if (c->next == NULL)
      return CHUNK_IN_USE;
   else
      return CHUNK_FREE;
}
/*--------------------------------------------------------------------*/
/* ChunkFirstFreeChunk                                                */
/* Returns the first chunk in the free chunk list. Returns NULL if    */
/* there is no free chunk.                                            */
/* -Parameter : None                                                  */
/* -Return value : (Chunk_T )gFreeHead                                */
/* -Global variable : gFreeHead (no modification)                     */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkFirstFreeChunk(void)
{
  return gFreeHead;
}
/*--------------------------------------------------------------------*/
/* ChunkLastFreeChunk                                                 */
/* Returns the last chunk in the free chunk list. Returns NULL if     */
/* there is no free chunk.                                            */
/* -Parameter : None                                                  */
/* -Return value : (Chunk_T) gFreeTail                                */
/* -Global variable : gFreeTail (no modification)                     */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkLastFreeChunk(void)
{
   return gFreeTail;
}
/*--------------------------------------------------------------------*/
/* ChunkNextFreeChunk                                                 */
/* Returns the next free chunk in free chunk list.                    */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) c->next                                  */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkNextFreeChunk(Chunk_T c)
{
  return c->next;
}
/*--------------------------------------------------------------------*/
/* ChunkInsertFirst                                                   */
/* Insert a chunk, 'c', into the head of the free chunk list.         */
/* It attempts to merge with the next adjacent chunk if possible.     */
/* -Parameter : Chunk_T c                                             */
/* -Return value : None                                               */
/* -Global variable : gFreeHead, gFreeTail (use and updates it)       */
/*--------------------------------------------------------------------*/
void
ChunkInsertFirst(Chunk_T c)
{
  Chunk_T footer, headfooter;
  
   /* get the addresse of footer*/
   footer = GetFooter(c);
   
   assert (ChunkGetStatus(c) != CHUNK_FREE);
   assert (c->units >= 1);
   assert (ChunkGetStatus(footer) != CHUNK_FREE);

   /* If free chunk list is empty, chunk c points to itself. */
   if (gFreeHead == NULL) {
      assert(gFreeTail == NULL);
      gFreeHead = c;
      gFreeTail = c;
      c->next = c;
      footer->next = c;
   }
   else {
      /* get footer of the headchunk */
      headfooter = GetFooter(gFreeHead);
      c->next = gFreeHead; /* now c is CHUNK_FREE */
      /* do adjustment */
      footer->next = gFreeTail;
      headfooter->next = c;
      gFreeTail->next = c;
      if (ChunkNextAdjacent(c) == gFreeHead) {
         ChunkMerge(c, gFreeHead);
      } 
      gFreeHead = c;
   }
   return;
}
/*--------------------------------------------------------------------*/
/* ChunkInsertAfter                                                   */
/* Insert a chunk, 'c', to the free chunk list after 'e' which is     */
/* already in the free chunk list. After the operation, 'c' will be   */
/* the next chunk of 'e' in the free chunk list.                      */
/* It attempts to merge with the previous chunk ('e') and with the    */
/* next chunk if possible. Returns the merged chunk.                  */
/* -Parameter : (Chunk_T) e, (Chunk_T) c                              */
/* -Return value : (Chunk_T) c                                        */
/* -Global variable : gFreeTail (use and updates it)                  */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkInsertAfter(Chunk_T e, Chunk_T c)
{
  Chunk_T n, footer, nextfooter;

  /* get footers' addresses */
  footer = GetFooter(c);
  nextfooter = GetFooter(e->next);
   
   assert (e < c);
   assert (ChunkGetStatus(e) == CHUNK_FREE);
   assert (ChunkGetStatus(c) != CHUNK_FREE);
   assert (ChunkGetStatus(GetFooter(e)) == CHUNK_FREE);
   assert (ChunkGetStatus(footer) != CHUNK_FREE);

   c->next = e->next; /* now c is CHUNK_FREE */
   e->next = c;
   footer->next = e;
   nextfooter->next = c;
   if (e == gFreeTail)
      gFreeTail = c;

   /* see if c can be merged with e */
   if (ChunkNextAdjacent(e) == c) 
      c = ChunkMerge(e, c);
   
   /* see if we can merge with n */
   n = ChunkNextAdjacent(c);
   if (n != NULL && ChunkGetStatus(n) == CHUNK_FREE)
      c = ChunkMerge(c, n);

   return c;
}
/*--------------------------------------------------------------------*/
/* ChunkGetUnits                                                      */
/* Returns the size of a chunk, 'c', in terms of the number of chunk  */
/* units.                                                             */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (int) c->units                                     */
/*--------------------------------------------------------------------*/
int
ChunkGetUnits(Chunk_T c)
{
   return c->units;
}
/*--------------------------------------------------------------------*/
/* ChunkNextAdjacent                                                  */
/* Returns the next adjacent chunk to 'c' in memory space.            */
/* Returns NULL if 'c' is the last chunk in memory space.             */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : NULL (c is last), (Chunk_T) n                      */
/* -Globl varialbe : gHeapEnd (no modification)                       */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkNextAdjacent(Chunk_T c)
{
   Chunk_T n;
   
   n = c + c->units + 2;
   
   /* If 'c' is the last chunk in memory space, then return NULL. */
   if ((void *)n >= gHeapEnd)
      return NULL;
   
   return n;
}
/*--------------------------------------------------------------------*/
/* ChunkMerge                                                         */
/* Merge two adjacent chunks and return the merged chunk.             */
/* Note that (1) c1 < c2, (2) they should be adjacent, and (3) they   */
/* should both be free chunks (with valid next pointers).             */
/* Returns the merged chunk pointer.                                  */
/* -Parameter : Chunk_T c1, Chunk_T c2                                */
/* -Return value : c1                                                 */
/* -Global variable : gFreeTail (use and updates it)                  */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkMerge(Chunk_T c1, Chunk_T c2)
{
  Chunk_T footer1, footer2, nextfooter;

  /* get footers' addresses*/
  footer1 = GetFooter(c1);
  footer2 = GetFooter(c2);
  nextfooter = GetFooter(c2->next);
  
   /* c1 and c2 must be be adjacent */
   assert (c1 < c2);
   assert (ChunkNextAdjacent(c1) == c2);
   /* check arguments are valid */
   assert (ChunkGetStatus(c1) == CHUNK_FREE);
   assert (ChunkGetStatus(c2) == CHUNK_FREE);
   assert (ChunkGetStatus(footer1) == CHUNK_FREE);
   assert (ChunkGetStatus(footer2) == CHUNK_FREE);

   /* Adjust the merged chunk */
   c1->units += (c2->units + 2);
   c1->next = c2->next;
   footer2->next = footer1->next;
   nextfooter->next = c1;

   /* Update tail if w2 was last chunk in free chunk list. */
   if (c2 == gFreeTail)
	   gFreeTail = c1;
   
   return c1;
}
/*--------------------------------------------------------------------*/
/* ChunkSplit                                                         */
/* Shrink 'c' to 'units' and return a new chunk with the reduced      */
/* space. Note that 'c' must be large enough to be split into two.    */
/* The status of the allocated chunk is set to CHUNK_IN_USE.          */
/* Returns the allocted chunk.                                        */
/* -Paremeter : (Chunk_T) c, (int) units                              */
/* -Return value : (Chunk_T) c2                                       */
/* -Global variable : gHeapStart, gHeapEnd (no modification)          */
/*--------------------------------------------------------------------*/
Chunk_T 
ChunkSplit(Chunk_T c, int units)
{
  Chunk_T c2, footer1, footer2;

  /* get to be splited chunk's address & footers' addresses */
   c2 = c + c->units - units;
   footer1 = c2 - 1;
   footer2 = c2 + units + 1;
   
   assert (c >= (Chunk_T)gHeapStart && c <= (Chunk_T)gHeapEnd);
   assert (ChunkGetStatus(c) == CHUNK_FREE);
   /* footer2 is same as the original footer of c */
   assert (ChunkGetStatus(footer2) == CHUNK_FREE);
   assert (c->units > units + 2); /* because of  new footer 
                                             and new header */

   /* prepare for the first chunk */
   c->units -= (units + 2);
   footer1->units = c->units;
   footer1->next = footer2->next;
   /* prepare for the second chunk */
   c2->units = units;
   c2->next = NULL; /* set next to NULL since it will be used soon */
   footer2->units = units;
   footer2->next = NULL;

   return c2;
}
/*--------------------------------------------------------------------*/
/* ChunkRemoveFromList                                                */
/* Removes 'c' from the free chunk list.                              */
/* prev should be the previous free chunk just before 'c'.            */
/* If 'c' is the first chunk, prev should be the last chunk.          */
/* If 'c' is the only free chunk in the list, prev should be 'c'      */
/* -Parameter : (Chunk_T) prev, (Chunk_T) c                           */
/* -Return value : None                                               */
/* -Global variable : gFreeHead, gFreeTail (use and updates it)       */
/*--------------------------------------------------------------------*/
void
ChunkRemoveFromList(Chunk_T prev, Chunk_T c)
{
  Chunk_T next, nextfooter, footer;

  /* get footers' addresses */
   footer = GetFooter(c);
   next = c->next;
   nextfooter = GetFooter(next);
  
   assert (ChunkGetStatus(c) == CHUNK_FREE);
   assert (ChunkGetStatus(footer) == CHUNK_FREE);
   
   if (prev == c) { 
      assert(c->next == c);
      /* c is the only chunk in the free list */
      gFreeHead = gFreeTail = NULL;
   }
   else {
     /* do adjustment */
      assert(c->next != c);
      prev->next = c->next;
      nextfooter->next = prev;
      if (c == gFreeHead)
         gFreeHead = c->next;
      else if (c == gFreeTail)
         gFreeTail = prev;
   }
   c->next = NULL; /* its status is now CHUNK_IN_USE */
   footer->next = NULL;
   return;
}
/*--------------------------------------------------------------------*/
/* ChunkAllocateMem                                                   */
/* Allocate a new chunk which is capable of holding 'units' chunk     */
/* units in memory by expanding the heap, and return the new          */
/* chunk. This function also performs chunk merging if possible after */
/* new chunk allocation.                                              */
/* -Parameter : (int) units                                           */
/* -Return value : (Chunk_T) c                                        */
/* -Global variable : gHeapEnd (updates it), gFreeTail (only use)     */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkAllocateMem(int units)
{
  Chunk_T c, footer;

   if (units < MEMALLOC_MIN)
      units = MEMALLOC_MIN;
   
   /* Note that we need to allocate two more unit 
                            for header and footer. */
   c = (Chunk_T)sbrk((units + 2) * CHUNK_UNIT);
   if (c == (Chunk_T)-1)
      return NULL;
   
   gHeapEnd = sbrk(0);
   
   footer = GetFooter(c);

   /* set new chunk */
   c->units = units;
   c->next = NULL;
   footer->units = units;
   footer->next = NULL;

   /* Insert the newly allocated chunk 'c' to the free list.
    * Note that the list is sorted in ascending order. */
   if (gFreeTail == NULL) {
      assert(gFreeHead == NULL);
      ChunkInsertFirst(c);
   }
   else {
     c = ChunkInsertAfter(gFreeTail, c);
   }
 
   assert(ChunkValidityCheck());

   return c;
}
/*--------------------------------------------------------------------*/
/* ChunkFreeChunk                                                     */
/* Mark a chunk as free by inserting it to free doubly linked list.   */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : None                                               */
/* -Global variable : gFreeHead (no modification)                     */
/*--------------------------------------------------------------------*/
void
ChunkFreeChunk(Chunk_T c)
{
  Chunk_T w, front, prev;
   
   w = ChunkFirstFreeChunk();

   /* If the free chunk list is empty or new chunk's address is less
    * than first chunk in free chunk list, insert at beginning. */
   if (w == NULL || c < w) {
      ChunkInsertFirst(c); 
      return;
   }

   assert(w < c); /* c's address must be larger than the first chunk */

   /* search next free chunk */
   for(front = ChunkNextAdjacent(c); front != NULL;
       front = ChunkNextAdjacent(front)){
     if(front->next != NULL){
       break;
     }
   }

   if(front == NULL)
     front = gFreeHead;
   
   prev = (GetFooter(front))->next;

   /* free c */
   ChunkInsertAfter(prev, c);

   return;
}

#ifndef NDEBUG

/*--------------------------------------------------------------------*/
/* ChunkIsValid                                                       */
/* Check each chunk is valid.                                         */
/* Returns 'true' (non-zero value) on valid, 'false' (zero) on        */
/* invalid.                                                           */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : 0 (invalid), 1 (valid)                             */
/* -Global variable : gHeapStart, gHeapEnd (no modification)          */
/*--------------------------------------------------------------------*/
static int 
ChunkIsValid(Chunk_T c)
/* Return 1 (TRUE) iff c is valid */
{
  Chunk_T footer;

  footer = GetFooter(c);
  
   assert(c != NULL);
   assert(gHeapStart != NULL);
   assert(gHeapEnd != NULL);

   if (c < (Chunk_T)gHeapStart)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (c >= (Chunk_T)gHeapEnd)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (c->units == 0)
      {fprintf(stderr, "Zero units in header\n"); return 0; }
   if(footer->units == 0)
     {fprintf(stderr, "Zero units in footer\n"); return 0; }
   return 1;
}

/*--------------------------------------------------------------------*/
/* ChunkValidityCheck                                                 */
/* Validity check for entire data structures for chunk.               */
/* Returns 'true' (non-zero value) on success, 'false' (zero) on      */
/* failure.                                                           */
/* -Parameter : None                                                  */
/* -Return value : 0 (failure), 1 (success)                           */
/* -Global variable : gFreeHead, gFreeTail, gHeapStart, gHeapTail     */
/*                    (no modificaiton)                               */
/*--------------------------------------------------------------------*/
int
ChunkValidityCheck(void)
{
   Chunk_T w;

   if (gHeapStart == NULL) {
      fprintf(stderr, "Uninitialized heap start\n");
      return 0;
   }

   if (gHeapEnd == NULL) {
      fprintf(stderr, "Uninitialized heap end\n");
      return 0;
   }

   if (gHeapStart == gHeapEnd) {
      if (gFreeHead == NULL)
         return 1;
      fprintf(stderr, "Inconsistent empty heap\n");
      return 0;
   }
   /* check all chunks are valid */
   for (w = (Chunk_T)gHeapStart; 
        w && w < (Chunk_T)gHeapEnd;
        w = ChunkNextAdjacent(w)) {
      if (!ChunkIsValid(w)) 
         return 0;
   }
   /* check free chunks more precisely */
   for (w = gFreeHead; w; w = w->next) {
     if (w->units != GetFooter(w)->units){
       fprintf(stderr, "Header and footer have different units\n");
       return 0;
     }
     if ((ChunkGetStatus(w) != CHUNK_FREE) &&
	 (ChunkGetStatus(w + w->units + 1) != CHUNK_FREE)) {
         fprintf(stderr, "Non-free chunk in the free chunk list\n");
         return 0;
      }
      if (ChunkNextAdjacent(w) == w->next) {
         fprintf(stderr, "Uncoalesced chunks\n");
         return 0;
      }
      if (w->next == gFreeHead)
	break;
   }

   return 1;
}
/*--------------------------------------------------------------------*/
/* ChunkPrint                                                         */
/* Print out entire data sturucture for chunks. This function will    */
/* print all chunks in dobuly linked list and in memory.              */
/* -Paremeter : None                                                  */
/* -Return value : None                                               */
/* -Global variable : gFreeHead, gHeapStart, gHeapEnd(no modificarion)*/
/*--------------------------------------------------------------------*/
void
ChunkPrint(void)
{
   Chunk_T w, p = NULL;
   
   printf("heap: %p ~ %p\n", gHeapStart, gHeapEnd);
   printf("gFreeHead: %p\n", (void *)gFreeHead);
   
   printf("free chain\n");
   for (w = gFreeHead; w; w = w->next) {
      if (p == w) {
	printf("%p->next == %p, weird!\n", 
	       (void *)p, (void *)w); exit(0);
      }
      printf("[%p: %ld units]\n", (void *)w, w->units);
      p = w;

      if (w->next == gFreeHead)
	break;
   }
   
   printf("mem\n");
   for (w = (Chunk_T)gHeapStart;
        w < (Chunk_T)gHeapEnd;
        w = (Chunk_T)((char *)w + (w->units + 1) * CHUNK_UNIT))
     printf("[%p (%c): %ld units]\n", 
	    (void *)w, w->next ? 'F' : 'U', w->units);
}
#endif
