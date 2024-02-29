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

#include "sync_utils.h"
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char *buf, size_t n)
{
  sprint("hartid = %d 1324\n", read_tp());
  sprint("hartid = %d : %s buf_addr:%x\n", read_tp(), buf, buf);
  sprint("hartid = %d 1324\n", read_tp());
  return 0;
}

//
// implement the SYS_user_exit syscall
//
int exit_counter = 0;
ssize_t sys_user_exit(uint64 code)
{
  sprint("hartid = : User exit with code:%d.\n", code);
  //  in lab1, PKE considers only one app (one process).
  //  therefore, shutdown the system when the app calls exit()
  //  if (read_tp())
  //    exit_counter++;
  //  else
  //  {
  //  sync_barrier(&exit_counter, NCPU);
  sprint("before:%d\n", read_tp());
  if (!read_tp())
  {
    sprint("hartid = %d: shutdown with code:%d.\n", read_tp(), code);
    shutdown(code);
  }
  else
  {
    sprint("hartid = %d wait\n", read_tp());
    sync_barrier(&exit_counter, NCPU);
  }
  //}
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
  default:
    panic("Unknown syscall %ld \n", a0);
  }
}
