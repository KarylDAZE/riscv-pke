#ifndef _SCHED_H_
#define _SCHED_H_

#include "process.h"

// length of a time slice, in number of ticks
#define TIME_SLICE_LEN 2

#define MAX_SEMS 10

void insert_to_ready_queue(process *proc);
void schedule();

extern int sems[MAX_SEMS];
#endif
