#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include <string.h>

static void print_errorline();

static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer()
{
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64 *)CLINT_MTIMECMP(cpuid) = *(uint64 *)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}

//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap()
{
  uint64 mcause = read_csr(mcause);
  print_errorline();
  switch (mcause)
  {
  case CAUSE_MTIMER:
    handle_timer();
    break;
  case CAUSE_FETCH_ACCESS:
    handle_instruction_access_fault();
    break;
  case CAUSE_LOAD_ACCESS:
    handle_load_access_fault();
  case CAUSE_STORE_ACCESS:
    handle_store_access_fault();
    break;
  case CAUSE_ILLEGAL_INSTRUCTION:
    // TODO (lab1_2): call handle_illegal_instruction to implement illegal instruction
    // interception, and finish lab1_2.

    handle_illegal_instruction();
    break;
  case CAUSE_MISALIGNED_LOAD:
    handle_misaligned_load();
    break;
  case CAUSE_MISALIGNED_STORE:
    handle_misaligned_store();
    break;

  default:
    sprint("machine trap(): unexpected mscause %p\n", mcause);
    sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
    panic("unexpected exception happened in M-mode.\n");
    break;
  }
}

static void print_errorline()
{
  char path[200], source_code[200], file_code[10000];
  uint64 eaddr = read_csr(mepc);
  // sprint("mepc:%x\n", read_csr(mepc));
  for (int i = 0;; i++)
  {
    if (eaddr == current->line[i].addr)
    {
      strcpy(path, current->dir[current->file[current->line[i].file].dir]);
      strcpy(path + strlen(path), "/");
      strcpy(path + strlen(path), current->file[current->line[i].file].file);
      sprint("Runtime error at %s:%d\n", path, current->line[i].line);

      // to find source code
      int line = current->line[i].line, source_code_offset = 0;
      spike_file_t *fp = spike_file_open(path, O_RDONLY, 0);
      spike_file_read(fp, file_code, 10000);
      while (--line)
      {
        while (file_code[source_code_offset++] != '\n')
          ;
      }
      int i;
      for (i = 0; file_code[source_code_offset] != '\n'; i++, source_code_offset++)
      {
        source_code[i] = file_code[source_code_offset];
      }
      source_code[i] = 0;
      sprint("%s\n", source_code);

      // sprint("line:%d\ndir:%s\nfile:%s", current->line[i].line, current->dir[current->file[current->line[i].file].dir], current->file[current->line[i].file].file);
      break;
    }
  }
  // sprint("dir:%s\nline:%x %d %d\nfile:%s\n", current->dir[current->file[0].dir], current->line[0].addr, current->line[0].file, current->line[0].line, current->file[0].file);
}