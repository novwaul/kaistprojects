/*--------------------------------------------------------------------*/
/* chunk2.h                                                           */
/* Assignment 5                                                       */
/* Author: In je Hwang                                                */
/* Student ID : 20160788                                              */
/*--------------------------------------------------------------------*/

#ifndef _CHUNK_BASE_H_
#define _CHUNK_BASE_H_

#pragma once

#include <stdbool.h>
#include <unistd.h>
#define BUCKET_SIZE 1024

typedef struct Chunk *Chunk_T;

extern Chunk_T bucket[BUCKET_SIZE];

enum {
   CHUNK_FREE,
   CHUNK_IN_USE,
};

enum {
   CHUNK_UNIT = 16, /* 16 = sizeof(struct Chunk) */
};
/*--------------------------------------------------------------------*/
/* ChunkRemoveFromList                                                */
/* Remove free chunk from its list and return the removed chunk in    */
/* unfree status.                                                     */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkRemoveFromList(Chunk_T c);

/*--------------------------------------------------------------------*/
/* ChunkInsert                                                        */
/* Insert unfree chunk in the multiple doubly linked list.            */
/* Return inserted chunk.                                             */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkInsert(Chunk_T c);

/*--------------------------------------------------------------------*/
/* ChunkNextAdjacent                                                  */
/* Return the address of next adjacent chunk.                         */
/* If there is no chunk next, return NULL.                            */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkNextAdjacent(Chunk_T c);

/*--------------------------------------------------------------------*/
/* ChunkAllocateMem                                                   */
/* Allocate memory for new chunk and return the chunk in unfree staus.*/
/* If cannot allocate it, return NULL.                                */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkAllocateMem(int units);

/*--------------------------------------------------------------------*/
/* ChunkSplit                                                         */
/* Split the chunk and return new-born chunk.                         */
/* If splited chunk units is over than 1023, it maintain its list.    */
/* If the chunk is under or same as 1023, it changes its list.        */
/*--------------------------------------------------------------------*/
Chunk_T 
ChunkSplit(Chunk_T c, int units);

/*--------------------------------------------------------------------*/
/* ChunkMerge                                                         */
/* Merge two chunks to one. Return the chunk unfree status.           */
/* If chunk is free, adjust its old list and remove it from the list. */
/* Parameter should be adjacnet and must be c1 < c2.                  */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkMerge(Chunk_T c1, Chunk_T c2);

/*--------------------------------------------------------------------*/
/* ChunkInitDS                                                        */
/* Initialize Heap data section.                                      */
/*--------------------------------------------------------------------*/
void
ChunkInitDS(void);

/*--------------------------------------------------------------------*/
/* ChunkFreeChunk                                                     */
/* Free unfree chunk. If the chunk can merge with adjacent chunks, it */
/* tries to merge.                                                    */
/*--------------------------------------------------------------------*/
void
ChunkFreeChunk(Chunk_T c);

/*--------------------------------------------------------------------*/
/* ChunkUnits                                                         */
/* Return units of the chunk.                                         */
/*--------------------------------------------------------------------*/
int
ChunkUnits(Chunk_T c);

/*--------------------------------------------------------------------*/
/* ChunkGetNext                                                       */
/* Return next free chunk in multiple doubly linked list.             */
/*--------------------------------------------------------------------*/
Chunk_T
ChunkGetNext(Chunk_T c);

/*--------------------------------------------------------------------*/
/* GetFooter                                                          */
/* Return the address of the footer of the chunk.                     */
/*--------------------------------------------------------------------*/
Chunk_T
GetFooter(Chunk_T c);

#ifndef NDEBUG

/*--------------------------------------------------------------------*/
/* ChunkValidityCheck                                                 */
/* Validity check for entire data structures for chunk.               */
/* Returns 'true' (non-zero value) on success, 'false' (zero) on      */
/* failure.                                                           */
/*--------------------------------------------------------------------*/
int
ChunkValidityCheck(void);

/*--------------------------------------------------------------------*/
/* ChunkPrint                                                         */
/* Print out entire data sturucture for chunks. This function will    */
/* print all chunks in multiple doubly linked list and in memory.     */
/*--------------------------------------------------------------------*/
void
ChunkPrint(void);

#endif /* NDEBUG */

#endif /* _CHUNK_BASE_H_ */
