#ifndef INCLUDE_SYSCALLS_H
#define INCLUDE_SYSCALLS_H

#include "../INTERRUPTS/interrupt.h"

#define SYSCALL_WRITE 1
#define SYSCALL_EXIT 2
#define SYSCALL_READ 3
#define SYSCALL_GETPID 4
#define SYSCALL_MAX 5

int syscall_entry(struct cpu_state *cpu);

#endif
