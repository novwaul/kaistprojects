/*--------------------------------------------------------------------*/
/* Assigenment 5                                                      */
/* chunk2.c                                                           */
/* Author: In Je Hwang                                                */
/* Student ID : 20160788                                              */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "chunk2.h"

enum {
   MEMALLOC_MIN = 1024,
};

struct Chunk {
   Chunk_T next;     /* Pointer to the next chunk in the free chunk list */
   size_t units;     /* Capacity of a chunk (chunk units) */
};

Chunk_T bucket[BUCKET_SIZE];/* BUCKET_SIZE == 1024 */

/* gHeapStart, gHeapEnd: start and end of the heap area.
 * gHeapEnd will move if you increase the heap */
static void *gHeapStart = NULL, *gHeapEnd = NULL;
/*--------------------------------------------------------------------*/
/* ChunkGetNext                                                       */
/* Return next free chunk in multiple doubly linked list.             */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) c->next                                  */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkGetNext(Chunk_T c)
{
  return c->next;
}
/*--------------------------------------------------------------------*/
/* ChunkUnits                                                         */
/* Return units of the chunk.                                         */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (int) c->units                                     */
/*--------------------------------------------------------------------*/
int
ChunkUnits(Chunk_T c)
{
  return c->units;
}
/*--------------------------------------------------------------------*/
/* GetFooter                                                          */
/* Return the address of the footer of the chunk.                     */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) footer                                   */
/*--------------------------------------------------------------------*/
Chunk_T
GetFooter(Chunk_T c)
{
  Chunk_T footer;

  assert(c != NULL);

  footer = c + c->units + 1;

  return footer;
}
/*--------------------------------------------------------------------*/
/* ChunkInitDS                                                        */
/* Initialize Heap data section.                                      */
/* -Parameter : None                                                  */
/* -Return value : None                                               */
/* -Global variable : gHeapStart, gHeapEnd (updates it)               */
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
/* ChunkInsert                                                        */
/* Insert unfree chunk in the multiple doubly linked list.            */
/* Return inserted chunk.                                             */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) c                                        */
/* -Global variable : bucket[] (use and updates it)                   */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkInsert(Chunk_T c)
{
  Chunk_T footer, bfooter;
  size_t index;

  assert (c != NULL);
  assert (c->next == NULL);/* Insert unfree chunk */
   
  /* get footer address */
  footer = GetFooter(c);

  assert (footer->next == NULL);

  /* get index */
  index = c->units - 1;
  if(c->units > BUCKET_SIZE)/* units over 1024 */
    index = BUCKET_SIZE - 1;/* store in bucket[1024] */

  /* do insert */
   if(bucket[index] == NULL){
     bucket[index] = c;
     c->next = c;
     footer->next = c;
   }
   else{
     /* prepare insert */
     bfooter = GetFooter(bucket[index]);
     /* do insert c */
     c->next = bucket[index];
     footer->next = bfooter->next;
     /* adjust inserted list */
     bfooter->next = c;
     footer->next->next = c;
     bucket[index] = c;/* insert front */
   }

   /* before return, check return value */
   if(c->units < BUCKET_SIZE)
     assert(index + 1 == c->units);
   if(c->units >= BUCKET_SIZE)
     assert(index + 1 <= c->units);
   assert(bucket[index] != NULL);
   assert(c->units == footer->units);
   assert(c->next != NULL);
   assert(footer->next != NULL);
   return c;
}
/*--------------------------------------------------------------------*/
/* ChunkNextAdjacent                                                  */
/* Return the address of next adjacent chunk.                         */
/* If there is no chunk next, return NULL.                            */
/* -Parameter : (Chunk_T) c , NULL                                    */
/* -Return value : (Chunk_T) n                                        */
/* -Global variable : gHeapEnd (only use)                             */
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
/* Merge two chunks to one. Return the chunk unfree status.           */
/* If chunk is free, adjust its old list and remove it from the list. */
/* Parameter should be adjacnet and must be c1 < c2.                  */
/* -Parameter : (Chunk) c1, (Chunk) c2                                */
/* -Return Value : (Chunk_T) c1                                       */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkMerge(Chunk_T c1, Chunk_T c2)
{

   /* c1 and c2 must be be adjacent */
   assert (c1 < c2);
   assert (ChunkNextAdjacent(c1) == c2);
   
   /* remove each chunk form old list */
   if((c1->next != NULL) && (GetFooter(c1)->next != NULL)){
     ChunkRemoveFromList(c1);
   }
   else if ((c1->next == NULL) && (GetFooter(c1)->next == NULL))
     ;
   else assert(0); /* should not happen */
   if((c2->next != NULL) && (GetFooter(c2)->next != NULL)){
     ChunkRemoveFromList(c2);
   }
   else if ((c2->next == NULL) && (GetFooter(c2)->next == NULL))
     ;
   else assert(0); /* should not happen */
   
   /* Adjust the merged chunk */
   c1->units += (c2->units + 2);
   GetFooter(c1)->units = c1->units;

   assert(c1->units == GetFooter(c1)->units);
   return c1;
}
/*--------------------------------------------------------------------*/
/* ChunkSplit                                                         */
/* Split the chunk and return new-born chunk.                         */
/* If splited chunk units is over than 1023, it maintain its list.    */
/* If the chunk is under or same as 1023, it changes its list.        */
/* -Parameter : (Chunk_T) c, (int) units                              */
/* -Return value : (Chunk_T) c2                                       */
/* -Global variable : gHeapStart, gHeapEnd (only use),                */
/*                    bucket[] (use and updates it)                   */
/*--------------------------------------------------------------------*/
Chunk_T 
ChunkSplit(Chunk_T c, int units)
{
  Chunk_T c2, footer1, footer2;
  int index;

   assert (c >= (Chunk_T)gHeapStart && c <= (Chunk_T)gHeapEnd);
   assert (c->next != NULL);
   assert (c->units > units + 2); /* because of new footer 
                                              and new header */

   /* get to be splited chunk's address & footers' addresses */
   c2 = c + c->units - units;
   footer2 = GetFooter(c);
   
   /* notice that footer2 is same as the original footer of c */
   assert (footer2->next != NULL);

   /* get old index */
   index = c->units - 1;
   if (c->units > BUCKET_SIZE)
     index = BUCKET_SIZE - 1;
   
   /* adjust first chunk */
   c->units -= (units + 2);
   footer1 = GetFooter(c);
   footer1->units = c->units;

   /* adjust bucket */
   if(c->units < BUCKET_SIZE){/* units under 1024 */
     if(c == bucket[index]){
       if((c->next == c) && (footer2->next == c))
	 bucket[index] = NULL;
       else if((c->next != c) && (footer2->next != c))
	 bucket[index] = c->next;
       else
	 assert(0); /* should not happen */
     }
   }
   
   /* remove splited chunk from old list */
   if(c->units < BUCKET_SIZE ){/* untis under 1024 */ 
     if((c->next != c) && (footer2->next != c)){
       footer2->next->next = c->next;
       GetFooter(c->next)->next = footer2->next;
     /* set next to NULL since ChunkInsert func treats unfree chunks */
       c->next = NULL;
       footer1->next = NULL;
     }
     else if((c->next == c) && (footer2->next == c)){
       c->next = NULL;
       footer1->next = NULL;
     }
     else
       assert(0); /* should not happen */
   }
   else
     footer1->next = footer2->next;/* units same & over 1024 */

   /* insert splited chunk in new list */
   assert(c->units == GetFooter(c)->units);
   if(c->units < BUCKET_SIZE )
     ChunkInsert(c);
   
   /* prepare for the second chunk to give it to user */
   c2->units = units;
   c2->next = NULL; /* set next to NULL since it will be used soon */
   footer2->units = units;
   footer2->next = NULL;

   return c2;
}
/*--------------------------------------------------------------------*/
/* ChunkRemoveFromList                                                */
/* Remove free chunk from its list and return the removed chunk in    */
/* unfree status.                                                     */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : (Chunk_T) c                                        */
/* -Global variable : bucket[] (use and updates it)                   */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkRemoveFromList(Chunk_T c)
{
  Chunk_T footer;
  size_t index;

   assert (c != NULL); 
   assert (c->next != NULL);
   
   /* get footers' addresses */
   footer = GetFooter(c);

   assert (footer->next != NULL);

   /* get index */
   index = c->units - 1;
   if(c->units > BUCKET_SIZE)
     index = BUCKET_SIZE - 1;
   
   /* adjust old list */
   if(c->next != c){
     GetFooter(c->next)->next = footer->next; /* adjust next footer */
     footer->next->next = c->next; /* adjust previous header */
   }
   
   /* adjust bucket */
   if(c == bucket[index]){
     if((c->next == c) && (footer->next == c)){
       bucket[index] = NULL;
     }
     else if((c->next != c) && (footer->next != c)){
       bucket[index] = c->next;
     }
     else
       assert(0); /* should not happen */
   }

   /* make chunk unfree */
   c->next = NULL;
   footer->next = NULL;

   /* check before return */
   assert(c->units == footer->units);
   assert(c->next == NULL);
   assert(footer->next == NULL);
   return c;
}
/*--------------------------------------------------------------------*/
/* ChunkAllocateMem                                                   */
/* Allocate memory for new chunk and return the chunk in unfree staus.*/
/* If cannot allocate it, return NULL.                                */
/* -Parameter : (int) units                                           */
/* -Return value : (Chunk_T) c, NULL                                  */
/* -Global variable : gHeapEnd (updates it)                           */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkAllocateMem(int units)
{
  Chunk_T c, footer;

   if (units < MEMALLOC_MIN)
      units = MEMALLOC_MIN;
   
   /* Note that we need to allocate two more unit 
      for header and footer*/
   c = (Chunk_T)sbrk((units + 2) * CHUNK_UNIT);
   if (c == (Chunk_T)-1)
      return NULL;
   
   gHeapEnd = sbrk(0);
   
   /* set information about chunk */
   c->units = units;
   c->next = NULL;
   footer = GetFooter(c);
   footer->units = units;
   footer->next = NULL;
   
   ChunkInsert(c);
   
   assert(ChunkValidityCheck());

   return c;
}
/*--------------------------------------------------------------------*/
/* ChunkFreeChunk                                                     */
/* Free unfree chunk. If the chunk can merge with adjacent chunks, it */
/* tries to merge.                                                    */
/* -Parameter : (Chunk_T) c                                           */
/* -Return value : None                                               */
/* -Global variable : gHeapStart (only use)                           */
/*--------------------------------------------------------------------*/
void
ChunkFreeChunk(Chunk_T c)
{
  Chunk_T n, prev_footer;

  assert(c != NULL);
  assert(c->next == NULL);
  assert(GetFooter(c)->next == NULL);

  /* check if c can merge with adjacent chunks*/
  n = ChunkNextAdjacent(c);/* with next chunk */
  if((n != NULL) && 
     ((n->next != NULL) /* n must be free, 
                           check next chunk header*/
      && (GetFooter(n)->next != NULL))){/* check next chunk footer */
    c = ChunkMerge(c, n);
  }
  prev_footer = c - 1; /* with previous chunk */
  if(prev_footer <= (Chunk_T)gHeapStart)
    prev_footer = NULL;
  if((prev_footer != NULL) &&
     ((prev_footer->next != NULL) &&/* prev chunk must be free, 
                                       check prev chunk footer*/
      
      (prev_footer->next->next->next != NULL))){/* check prev chunk 
                                                   header*/
    n = prev_footer - prev_footer->units - 1;
    c = ChunkMerge(n, c);
  }

  ChunkInsert(c);
  
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
  
   assert(c != NULL);
   assert(gHeapStart != NULL);
   assert(gHeapEnd != NULL);

   footer = GetFooter(c);

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
  int i;

   if (gHeapStart == NULL) {
      fprintf(stderr, "Uninitialized heap start\n");
      return 0;
   }

   if (gHeapEnd == NULL) {
      fprintf(stderr, "Uninitialized heap end\n");
      return 0;
   }

   if (gHeapStart == gHeapEnd) {
     for(i = 0 ; i< BUCKET_SIZE ; i++ ){
       if (bucket[i] != NULL){
	 fprintf(stderr, "Inconsistent empty heap\n");
	 return 0;
       }
     }
   }
   /* check all chunks are valid */
   for (w = (Chunk_T)gHeapStart; 
        w && w < (Chunk_T)gHeapEnd;
        w = ChunkNextAdjacent(w)) {
      if (!ChunkIsValid(w)) 
         return 0;
   }
   /* check free chunks more precisely */
   for(i = 0 ; i < BUCKET_SIZE; i++){
     for (w = bucket[i]; w; w = w->next) {
       /* check units of free chunks */
       if(i != BUCKET_SIZE - 1){/* for units under 1024 */
	 if(w->units != i + 1){
	   fprintf(stderr, "Wrong units\n");
	   return 0;
	 }
       }
       else{/* for rest units*/
	 if(w->units < BUCKET_SIZE){
	   fprintf(stderr, "Wrong units\n");
	   assert(0);//
	   return 0;
	 }
       }
       /* check header and footer has same units */
       if(w->units != GetFooter(w)->units){
	  fprintf(stderr, "Stored units are different\n");
	  return 0;
	}
       /* check chunk is free */
       if ((w->next == NULL) ||
	 (GetFooter(w)->next == NULL)) {
         fprintf(stderr, "Non-free chunk in the free chunk list\n");
         return 0;
       }
       /* check coalescing */
        if (ChunkNextAdjacent(w) == w->next) {
         fprintf(stderr, "Uncoalesced chunks\n");
         return 0;
	}
	/* get out if check is over */
        if (w == GetFooter(bucket[i])->next)
	  break;
     }
   }

   return 1;
}
/*--------------------------------------------------------------------*/
/* ChunkPrint                                                         */
/* Print out entire data sturucture for chunks. This function will    */
/* print all chunks in multiple doubly linked list and in memory.     */
/* -Paremeter : None                                                  */
/* -Return value : None                                               */
/* -Global variable : gFreeHead, gHeapStart, gHeapEnd(no modificarion)*/
/*--------------------------------------------------------------------*/
void
ChunkPrint(void)
{
  Chunk_T w;
  int i;
   
   printf("heap: %p ~ %p\n", gHeapStart, gHeapEnd);
   
   printf("free chunks\n");
   for (i = 0; i < BUCKET_SIZE; i++) {
      w = bucket[i];
      printf("doubly linked list %d\n", i);
      while(1){
        printf("[%p: %ld units]\n", (void *)w, w->units);
        w = w->next;
	if(w == bucket[i])
	  break;
      }
   }
   
   printf("mem\n");
   for (w = (Chunk_T)gHeapStart;
        w < (Chunk_T)gHeapEnd;
        w = (Chunk_T)((char *)w + (w->units + 2) * CHUNK_UNIT))
     printf("[%p (%c): %ld units]\n", 
	    (void *)w, w->next ? 'F' : 'U', w->units);
}
#endif
