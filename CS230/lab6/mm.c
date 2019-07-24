/*
 * mm.c 
 * 
 * 20160788 InJe Hwang
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* single word size */
#define SWORD 4

/* double woed size */
#define DWORD 8

/* blockList size */
#define BLOCK_LIST_SIZE 10

/* get size */
#define GET_SIZE(ptr) (*((unsigned int *)(ptr)) & (~0x7))

/* set size */
#define SET_HSIZE(ptr, size) (*((unsigned int*)(ptr)) = size)
#define SET_TSIZE(ptr, size) (*((unsigned int *)((ptr) + size + 12)) = size)

/* set allocation bit*/
#define SET_HALLOC(ptr) (*(unsigned int *)(ptr) |= 0x1)
#define SET_TALLOC(ptr) (*(unsigned int *)((ptr) + GET_SIZE(ptr) + 12) |= 0x1)

/* set next */
#define SET_NEXT(ptr, next) (*((char**)((ptr) + 4)) = next)

/* get next */
#define GET_NEXT(ptr) (*(char**)((ptr) + 4))

/* set prev */
#define SET_PREV(ptr, prev) (*((char**)((ptr) + GET_SIZE(ptr) + 4)) = prev) 

/* get prev */
#define GET_PREV(ptr) (*(char**)((ptr) + GET_SIZE(ptr) + 4))

/* get original pointer */
#define GET_ORIGIN(ptr) (ptr - 8)

/* get adjacent next */
#define GET_ADJNEXT(ptr) ((ptr) + GET_SIZE(ptr)+ 16)

/* get adjacent prev */
#define GET_ADJPREV(ptr) ((ptr) - (*(unsigned int*)((ptr) - 4) & ~(0x7) ) - 16)

/* get allocation bit */
#define GET_ALLOC(ptr) (*(unsigned int *)(ptr) & 0x1)

/* get split pointer */
#define GET_SPLIT(ptr, size) ((ptr) + size + 16)

/* adjust return pointer */
#define RETURN(ptr) (void*)(ptr + 8);

/* static scalar varialbe */
static char **blockList; // store different size of blocks
static size_t alloc_block_size = 0; // counts total allocated bytes. only for debug

/*
 * mm_insert - set header and footer info in a block.
 *	       then insert the block into the list next to the root.
 */
static void mm_insert(char** blockList , char* block_ptr, unsigned int size)
{
	SET_HSIZE(block_ptr, size);
	SET_TSIZE(block_ptr, size);
	SET_PREV(block_ptr, NULL);
	/* find index */
	int index = 0;
	while(1)
	{
		if(size <= (DWORD << index)) break;
		else if(++index > 8) break;
	}

	char * next = blockList[index];
	SET_NEXT(block_ptr, next);
	blockList[index] = block_ptr;
	if(next != NULL) // if next is not NULL
	{
		SET_PREV(next, block_ptr);
	}
	return;
}

/*
 * mm_delete - delete a block from blockList.
 */
static void mm_delete(char** blockList, char * block_ptr)
{
	unsigned int size = GET_SIZE(block_ptr);
	char * next = GET_NEXT(block_ptr);
	char * prev = GET_PREV(block_ptr);
	/* set prev and next */
	if(next != NULL){ // if next is not NULL
		SET_PREV(next, prev);
	}
	if(prev != NULL){ // if prev is not NULL
		SET_NEXT(prev, next);
	}
	else{ //delete first block
		int index = 0;
		while(1){
			if(size <= (DWORD << index)) break;
			else if((++index) > 8) break;
		}
		blockList[index] = next;
	}
	return;
}

/*
 * mm_check - check the below properties.
	1. check free block is marked as free(0)
	2. check free block list contains proper blocks
	3. check coalescing
	4. check every free blocks are in the list
 */
static int mm_check()
{
	char * cur;
	unsigned int size, size_ref = DWORD, free_block_size = 0;
	/* go through blockList */
	int i;
	for(i = 0 ; i < BLOCK_LIST_SIZE ; i++)
	{
		cur = blockList[i];
		while(cur) // until NULL
		{
			size = GET_SIZE(cur);
			/* allocation bit check */
			if(GET_ALLOC(cur))
			{
				fprintf(stderr, "ERROR: in blockList[%d], allocation bit is set!\n", i);
				return 0;
			}
			/* size check */
			else if( ((i != BLOCK_LIST_SIZE - 1) && (size > size_ref)) || ((i == BLOCK_LIST_SIZE - 1) && (size <= (1 << (BLOCK_LIST_SIZE - 2)) )))
			{
				fprintf(stderr, "ERROR: in blockList[%d], size mismatched in header (%d) is found!\n", i, size);
				return 0;	
			}
			/* coalescing check */
			else if(cur != ((char*)mem_heap_lo()) + DWORD*BLOCK_LIST_SIZE) // check adjacent previous block
			{
				if(!GET_ALLOC(GET_ADJPREV(cur)))
				{
					fprintf(stderr, "ERROR: in blockList[%d], coalescing check failed with adjacent previous block\n", i);
					return 0;
				}
			}
			else if((cur + size + 2*DWORD) != mem_heap_hi() + 1) // check adjacnet next block
			{
				if(!GET_ALLOC(GET_ADJNEXT(cur)))
				{
					fprintf(stderr, "ERROR: in blockList[%d], coalescing check failed with adjacent next block\n", i);
					return 0;
				}
			}
			cur = GET_NEXT(cur); // update cur
			free_block_size += (size + 2*DWORD);
		}
		size_ref <<= 1;
	}
	/* check every free block is in the list */
	unsigned int rs, hs;
	if((rs = free_block_size + alloc_block_size + DWORD*BLOCK_LIST_SIZE ) != (hs = mem_heapsize()))
	{
		fprintf(stderr, "ERROR: free block missing - reachable size (%u), heap size (%u)\n", rs, hs);
		return 0;
	}
	return 1; //success
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	/* allocate haep for blockList */
	if((blockList = mem_sbrk(BLOCK_LIST_SIZE * DWORD)) == (void*)(-1)) 
		return -1;
	/* initialize the blockList */
	int i;
	for(i = 0 ; i < BLOCK_LIST_SIZE ; i++)
		blockList[i] = NULL;
	return 0;
}

/* 
 * mm_malloc - try to find proper free block in blockList.
 *	if find successfully, split it if possible and give free block.
 *	if cannot find proper free block, allocate new block using mem_sbrk
 *     
 */
void *mm_malloc(size_t size)
{
	/* find index */
	unsigned int ref_index, index = 0;
	unsigned int blockSize, refSize = DWORD;
	char * cur;
	int newsize = ALIGN(size); // arbitary 8 aligned number is possible
	while(index < BLOCK_LIST_SIZE - 1)
	{
		if(newsize <= refSize) break;
		else
		{
			refSize <<= 1;
			index++;
		}
	}
	ref_index = index;
	/* find free block */
	for(    ; index < BLOCK_LIST_SIZE ; index++)
	{
		cur = blockList[index];
		while(cur)
		{
			blockSize = GET_SIZE(cur);
			if(blockSize >= newsize) // check the block has enough bytes
			{
				if(blockSize - newsize >= DWORD + 2*DWORD) // check whether splitting is possible
				{
					unsigned int remainSize = blockSize - newsize;
					mm_delete(blockList, cur); // delete from free list
					/* insert splitted block */
					mm_insert(blockList, GET_SPLIT(cur, newsize), remainSize - 2*DWORD);
					/* set size and allocation bit for return */
					SET_HSIZE(cur, newsize);
					SET_HALLOC(cur);
					SET_TSIZE(cur, newsize);
					SET_TALLOC(cur);
					return RETURN(cur);
				}
				else // cannot split, give all block
				{
					mm_delete(blockList, cur); //delete from free list
					SET_HALLOC(cur);
					SET_TALLOC(cur);
					return RETURN(cur);
				}
			}
			else cur = GET_NEXT(cur);
		}
	}	
	/* allocate new block */
	char * ret;
	unsigned int allocSize;
	if(ref_index != BLOCK_LIST_SIZE - 1) allocSize = DWORD << ref_index;
	else allocSize = newsize;
	if((ret = mem_sbrk(allocSize + 2*DWORD)) == (void *)(-1)){
		return (void *)(-1);
	}
	SET_HSIZE(ret, allocSize);
	SET_HALLOC(ret);
	SET_TSIZE(ret, allocSize);
	SET_TALLOC(ret);
	return RETURN(ret); 
}

/*
 * mm_free - Freeing a block. 
 *	if coalescing is possible, it performs coalescing.
 */
void mm_free(void *ptr)
{
	char * cur = GET_ORIGIN(ptr);
	unsigned int size;
	size = GET_SIZE(cur);
	/* coalescing */
	if( cur != ((char*)mem_heap_lo() + BLOCK_LIST_SIZE*DWORD) && (GET_ALLOC(GET_ADJPREV(cur)) & 1) != 1)//check allocation bit of adjacent previous block 
	{
		size += (GET_SIZE(GET_ADJPREV(cur)) + 2*DWORD);
		mm_delete(blockList, GET_ADJPREV(cur));
		cur = GET_ADJPREV(cur);
		SET_HSIZE(cur, size);
		SET_TSIZE(cur, size);
	}
	if(GET_ADJNEXT(cur) != ((char*)mem_heap_hi() + 1) && GET_ALLOC(GET_ADJNEXT(cur)) != 1)//check allocation bit of adjacent next block
	{
		mm_delete(blockList, cur + size + 2*DWORD );
		size += ( GET_SIZE(GET_ADJNEXT(cur)) + 2*DWORD);
		SET_HSIZE(cur, size);
		SET_TSIZE(cur, size);
	}
	/* insert */
	mm_insert(blockList, cur, size);
	return;
}

/*
 * mm_realloc - if size is smaller than original one, check whether it can be splitted, and return original pointer.
 *	if size is larger, then check whether the original block can be expanded. if not, allocate new block using mem_sbrk
 */
void *mm_realloc(void *ptr, size_t size)
{
	if(ptr == NULL) 
	{
		return mm_malloc(size);
	}
	else if(size == 0) 
	{
		mm_free(ptr);
		return NULL;
	}

	char * cur = GET_ORIGIN(ptr);
	unsigned int newsize = ALIGN(size);
	unsigned int remainSize, blockSize = GET_SIZE(cur);
	
	if(newsize <= blockSize) // shrink current block
	{
		if((remainSize = blockSize - newsize) >= DWORD + 2*DWORD) //check split is possible
		{
			mm_insert(blockList, GET_SPLIT(cur, newsize), remainSize - 2*DWORD);
			SET_HSIZE(cur, newsize);
			SET_TSIZE(cur, newsize);
			SET_HALLOC(cur);
			SET_TALLOC(cur);
			return ptr;
		}
		else // cannot split
		{
			return ptr;
		}
	}
	else 
	{
		// check whether can extend current block
		if( GET_ADJNEXT(cur) == (mem_heap_hi() + 1) )
		{
			unsigned int expendSize = newsize - blockSize;
			char * expend;
			if((expend = mem_sbrk(expendSize)) == (void*)(-1))
				return (void *)(-1);
			SET_HSIZE(cur, newsize);
			SET_TSIZE(cur, newsize);
			SET_HALLOC(cur);
			SET_TALLOC(cur);
			return ptr;
		}
		// cannot extned. call mm_malloc
		char head_val[4], tail_val[4];
		memcpy(head_val, ptr, 4);
		memcpy(tail_val, ptr + blockSize - SWORD, 4);
		mm_insert(blockList, cur, blockSize);
		void * ret = mm_malloc(newsize);
		memcpy(ret, head_val, 4);
		memcpy(ret + 4, ptr + SWORD, blockSize - DWORD);
		memcpy(ret + blockSize - 4, tail_val, 4);
		return ret;
	}
}














