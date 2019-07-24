#ifndef VM_FRAME_H
#define VM_FRAME_H
#include "threads/synch.h"
#include "vm/page.h"
#include <list.h>
#include <hash.h>

struct frame_table_entry
{
	uint8_t* frame;			/* Physical address, */
	struct thread *owner;		/* Owner of this frame. */
	struct hash_elem fte_elem;	/* hash_elem of framn table entry*/
        struct page_table_entry *spte;  /* Supplement table entry for owner. */
       	int64_t count;			/* # of frame access. */
	bool fixed;			/* Indicate if frame is fixed. */
};

struct frame_table
{
        struct list delete_list;	/* List for frame table entries that will be deleted form the table. */
	struct hash fte_hash;		/* Frame table. */
        struct lock ft_lock;            /* Lock for access frame_table */
};

struct delete_list_entry
{
  struct hash_elem *h;			/* Frame table entry hash_elem. */
  struct list_elem dle_elem;
};

void frame_init (void);
bool allocate_frame (void *k_va, void *spte, bool on);
void delete_frame (struct page_table_entry *spte);
void frame_update_status (void);
struct page_table_entry* frame_find_evict (void);
void unfix (void *addr);
void fix (void *addr);
void *stack_grow (void *addr, bool on);
#endif /* vm/frame.h */
