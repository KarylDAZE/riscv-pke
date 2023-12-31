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

#include "spike_interface/spike_utils.h"

#include "elf.h"
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char *buf, size_t n)
{
  sprint(buf);
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

//
// implement the SYS_user_print_backtrace syscall
//
ssize_t sys_user_print_backtrace(unsigned long backStep)
{
  uint64 fp = current->trapframe->regs.s0 + 8, ra = current->trapframe->regs.ra;
  int lenToFunc, lenToFuncMin = INT32_MAX, i, minI;
  char funcName[100];
  do
  {
    fp = *(uint64 *)(fp - 16);
    ra = *(uint64 *)(fp - 8);
    // find in which func is ra
    for (i = 0, lenToFuncMin = INT32_MAX; i < 100; i++)
    {
      lenToFunc = ra - symtab[i].st_value;
      if ((symtab[i].st_info & 0xf) == 2 && lenToFunc > 0 && lenToFunc < lenToFuncMin) // symtab[i].st_info&0xf==2 means symbal type is func
      {
        lenToFuncMin = lenToFunc;
        minI = i;
      }
    }
    strcpy(funcName, strtab + symtab[minI].st_name);
    // sprint("ra:%x\n", (uint32)ra);
    sprint("%s\n", funcName);
    backStep--;
  } while (strcmp(funcName, "main") != 0 && backStep > 0);
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
  case SYS_user_print_backtrace:
    return sys_user_print_backtrace(a1);
  default:
    panic("Unknown syscall %ld \n", a0);
  }
}
