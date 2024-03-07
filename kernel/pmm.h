#ifndef _PMM_H_
#define _PMM_H_

#include "config.h"

// Initialize phisical memeory manager
void pmm_init();
// Allocate a free phisical page
void *alloc_page();
// Free an allocated page
void free_page(void *pa);
// lock when alloc or free page
void spin_lock(int *lock);
// unlock when alloc or free end
void spin_unlock(int *lock);

extern int vm_alloc_stage[NCPU];

#endif