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
  sprint("sp:%x fp:%x ra:%x\n", (uint32)current->trapframe->regs.sp, (uint32)current->trapframe->regs.s0, (uint32)current->trapframe->regs.ra);
  sprint("pre fp:%x\n", (uint32) * ((uint64 *)(current->trapframe->regs.s0 - 8)));
  sprint("ppre fp:%x\n", (uint32) * ((uint64 *)(*(uint64 *)(current->trapframe->regs.s0 - 8)) - 2));
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
