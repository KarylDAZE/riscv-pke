/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char *buf, size_t n)
{
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert(current);
  char *pa = (char *)user_va_to_pa((pagetable_t)(current->pagetable), (void *)buf);
  sprint(pa);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code)
{
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process).
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

typedef struct mem_alloc_block
{
  int offset;
  size_t size;
  struct mem_alloc_block *next;
} alloc_block;
static alloc_block allocated_page_list_s = {4096, 0, NULL}, begin_block = {0, 0, NULL};
static alloc_block *allocated_block_list = &allocated_page_list_s;
static uint64 page_va = 0;
//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(size_t alloc_size)
{
  // search every free block
  alloc_block *block = allocated_block_list, *p_block = &begin_block;
  if (page_va)
    do
    {
      // if space is enough to allocate
      // sprint("list: off:%d size:%d addr:%x next:%x\n", allocated_block_list->offset, allocated_block_list->size, allocated_block_list, allocated_block_list->next);
      if (block->offset - p_block->offset - (int)p_block->size >= (int)alloc_size + (int)sizeof(alloc_block))
      // insert a new block
      {
        alloc_block *new_block = (alloc_block *)ROUNDUP(current->heap_top, sizeof(alloc_block));
        current->heap_top += alloc_size + sizeof(alloc_block);
        p_block->next = new_block;
        new_block->offset = p_block->offset + p_block->size;
        new_block->next = block;
        new_block->size = alloc_size;
        if (block == allocated_block_list)
          allocated_block_list = new_block;
        // alloc_block *iblock = allocated_block_list;
        // while (iblock)
        // {
        //   sprint("iblock: off:%d size:%d addr:%x next:%x\n", iblock->offset, iblock->size, iblock, iblock->next);
        //   iblock = iblock->next;
        // }

        return page_va + new_block->offset;
      }
      p_block = block;
      block = block->next;
    } while (block);

  //  allocate a new page
  void *pa = alloc_page();
  current->heap_top = (uint64)pa;
  page_va = g_ufree_page;
  g_ufree_page += PGSIZE;
  user_vm_map((pagetable_t)current->pagetable, page_va, PGSIZE, (uint64)pa,
              prot_to_type(PROT_WRITE | PROT_READ, 1));
  alloc_block *new_block = (alloc_block *)current->heap_top;
  current->heap_top += alloc_size + sizeof(alloc_block);
  new_block->offset = 0;
  new_block->size = alloc_size;
  new_block->next = allocated_block_list;
  allocated_block_list = new_block;
  return page_va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va)
{
  alloc_block *block = allocated_block_list, *p_block = allocated_block_list;
  if (0 == allocated_block_list->size)
    sprint("error free!\n");
  if (page_va)
    // search block to free
    while (block)
    {
      if (page_va + block->offset == va)
      {
        // sprint("free va:%x\n", va);
        if (block == allocated_block_list)
        {
          allocated_block_list = block->next;
        }
        else
        {
          p_block->next = block->next;
        }
        break;
      }
      p_block = block;
      block = block->next;
    }
  if (NULL == allocated_block_list->next)
  {
    user_vm_unmap((pagetable_t)current->pagetable, page_va, PGSIZE, 1);
    page_va = 0;
  }
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  switch (a0)
  {
  case SYS_user_print:
    return sys_user_print((const char *)a1, a2);
  case SYS_user_exit:
    return sys_user_exit(a1);
  // added @lab2_2
  case SYS_user_allocate_page:
    return sys_user_allocate_page(a1);
  case SYS_user_free_page:
    return sys_user_free_page(a1);
  default:
    panic("Unknown syscall %ld \n", a0);
  }
}
