#ifndef INCLUDE_SYSCALLS_H
#define INCLUDE_SYSCALLS_H

#include "../INTERRUPTS/interrupt.h"

#define SYSCALL_INTERRUPT_VECTOR 128

#define SYSCALL_EXIT 1
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_GETPID 20

#define SYSCALL_FS_CREATE 450
#define SYSCALL_FS_WRITE 451
#define SYSCALL_FS_READ 452
#define SYSCALL_FS_MKDIR 453
#define SYSCALL_FS_LIST 454

int syscall_dispatch(struct cpu_state *cpu);

#endif
