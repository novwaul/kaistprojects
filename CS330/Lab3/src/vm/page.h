#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <filesys/off_t.h>
#include "threads/synch.h"

#define EXEC 1
#define MMAP 2

struct page_table_entry 
{
	uint8_t* uvaddr;		/* Virtual address. */
        uint8_t* frame;			/* Physical address. */
        bool writable;			/* Is this page writable? */
        bool exist;			/* Does this page exist in frame table? */
	bool exist_in_swap_disk;	/* Does this page exist in swap slot? */
        bool modified;			/* Does this page modified? */
	struct hash_elem spte_elem;	/* Hash element for supplment page table. */
        struct file *file;		/* File. */
        off_t read_size;		/* Read bytes form FILE. */
	off_t of;			/* File offset. */
	uint32_t type;			/* Type of data. */
	size_t sector;			/* The first sector # where the data is in swap slot. */
        struct lock in_process_lock;	/* Lock for swaping & deletion. */ 
};

bool sup_page_table_init (void);
struct page_table_entry *allocate_mmap (void *addr,
					void *frame,
					struct file *file,
					off_t rs,
					off_t of
					);
struct page_table_entry *allocate_page (void *addr, 
					void* frame, 
					bool writable, 
					struct file *file, 
					off_t rs,
					off_t of
					);
struct page_table_entry *deallocate_page (void *addr);
bool is_writable (void *addr);
bool is_exist (void *addr);
struct page_table_entry *find_spte (void *addr);
void destroy_pages (void);

#endif /* vm/page.h */
