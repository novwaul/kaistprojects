#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "vm/page.h"

void swap_init (void);
bool swap_in (void *addr, bool on);
void *swap_out (void);
void swap_slot_delete (struct page_table_entry *spte);

#endif /* vm/swap.h */
