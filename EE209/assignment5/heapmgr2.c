/*--------------------------------------------------------------------*/
/* heapmgr2.c                                                         */
/* Assignment 5                                                       */
/* Author: In Je Hwang                                                */
/* Student ID : 20160788                                              */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <assert.h>

#include "chunk2.h"


/*--------------------------------------------------------------*/
/* SizeToUnits:
 * Returns capable number of units for 'size' bytes. */
/*--------------------------------------------------------------*/
int
SizeToUnits(int size)
{
   return (size + (CHUNK_UNIT-1))/CHUNK_UNIT;
}
/*--------------------------------------------------------------*/
/* GetChunkFromDataPtr:
 * Returns the header pointer that contains data 'm'. */
/*--------------------------------------------------------------*/
Chunk_T
GetChunkFromDataPtr(void *m)
{
   return (Chunk_T)((char *)m - CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* mymalloc:
 * Dynamically allocate a memory capable of holding size bytes. 
 * Substitute for GNU malloc().                                 */
/*--------------------------------------------------------------*/
void *
mymalloc(size_t size)
{
   static int isInitialized = 0;
   Chunk_T c;
   int units, index, proper;
   
   /* check size */
   if (size <= 0)
      return NULL;
   
   /* initialize */
   if (isInitialized == 0) {
      ChunkInitDS();
      isInitialized = 1;
   }

   assert(ChunkValidityCheck());

   /* get index */
   units = SizeToUnits(size);
   index = units - 1;
   if(units > BUCKET_SIZE)
     index = BUCKET_SIZE - 1;

   /* get proper value to split*/
   proper = units + 3;
   
   while(1){
   /* units same or under 1023 (under 1024) */
   if(units <= 1023){
     /* find best fit */
     if((c = bucket[index]) != NULL){
       c = ChunkRemoveFromList(c);
       assert(ChunkUnits(c) == ChunkUnits(GetFooter(c)));
       assert(ChunkGetNext(c) == NULL);
       assert(ChunkGetNext(GetFooter(c)) == NULL);
       return (char*)c + CHUNK_UNIT;
     }
     /* find chunk to split in the top list */
     for(c = bucket[BUCKET_SIZE - 1]; ; ){
       if(c == NULL)
	 break;
       if(ChunkUnits(c) >= proper){
	 c = ChunkSplit(c, units);
	 assert(ChunkUnits(c) == ChunkUnits(GetFooter(c)));
	 assert(ChunkGetNext(c) == NULL);
	 assert(ChunkGetNext(GetFooter(c)) == NULL);
	 return (char*)c + CHUNK_UNIT;
       }
       c = ChunkGetNext(c);
       if(c == bucket[BUCKET_SIZE - 1])
	 break;
	 }
     /* allocate memory */
     ChunkAllocateMem(units);
     continue;
   }
   /* units over 1023 (over or same 1024)*/
   else{
     /* no chunks in top list */
     if((c = bucket[BUCKET_SIZE - 1]) == NULL){
       ChunkAllocateMem(units);
       continue;
     }
     /* find proper chunk */
     while(1){
       if(ChunkUnits(c) == units){
	 c = ChunkRemoveFromList(c);
	 assert(ChunkUnits(c) == ChunkUnits(GetFooter(c)));
	 assert(ChunkGetNext(c) == NULL);
	 assert(ChunkGetNext(GetFooter(c)) == NULL);
	 return (char*)c + CHUNK_UNIT;
       }
       if(ChunkUnits(c) >= proper){
	 c = ChunkSplit(c, units);
	 assert(ChunkUnits(c) == ChunkUnits(GetFooter(c)));
	 assert(ChunkGetNext(c) == NULL);
	 assert(ChunkGetNext(GetFooter(c)) == NULL);
	 return (char*)c + CHUNK_UNIT;
       }
       c = ChunkGetNext(c);
       if(c == bucket[BUCKET_SIZE - 1])
	 break;
     }
     /* allocate memory */
     ChunkAllocateMem(units);
     continue;
   }
   }
}
/*--------------------------------------------------------------*/
/* myfree:
 * Releases dynamically allocated memory. 
 * Substitute for GNU free().                                   */
/*--------------------------------------------------------------*/
void
myfree(void *m)
{
   Chunk_T c;

   if (m == NULL)
      return;

   /* Check everything is OK before freeing 'm' */
   assert(ChunkValidityCheck());

   /* Get the chunk header pointer from m */
   c = GetChunkFromDataPtr(m);
   assert (ChunkGetNext(c) == NULL);

   /* Insert the chunk into the free chunk list */
   ChunkFreeChunk(c);
   /* Check everything is OK after freeing 'm' */
   assert(ChunkValidityCheck());

   return;
}
