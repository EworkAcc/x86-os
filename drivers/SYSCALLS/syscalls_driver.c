#include "syscalls.h"
#include "../SP/sp.h"
#include "../INTERRUPTS/kb.h"

#define USER_MEMORY_LIMIT 0x00400000

typedef int(*syscall_handler_t)(struct cpu_state *cpu);

static int user_buffer_validity(unsigned int address, unsigned int length) {
  if(length == 0) {
    return 1;
  }
  if(address == 0) {
    return 0;
  }

  unsigned int end = address + length - 1;
  if(end < address) {
    return 0;
  }
  if(end >= USER_MEMORY_LIMIT) {
    return 0;
  }

  return 1;
}

static int syscall_write(struct cpu_state *cpu) {
  if(!user_buffer_validity(cpu->ebx, cpu->ecx)) {
    return -1;
  }
  serial_write((char *)cpu->ebx, cpu->ecx);
  return (int)cpu->ecx;
}
static int syscall_exit(struct cpu_state *cpu __attribute__((unused))) {
  serial_write("User program exit\n", 18);
  for(;;) {
    asm volatile("cli; hlt");
  }
  return 0;
}
static int syscall_read(struct cpu_state *cpu) {
  if(!user_buffer_validity(cpu->ebx, cpu->ecx)) {
    return -1;
  }
  if(cpu->ecx == 0) {
    return 0;
  }

  char *buffer = (char *)cpu->ebx;
  unsigned int i;
  for(i = 0; i < cpu->ecx; ++i) {
    char c = 0;
    while(c == 0) {
      unsigned char scan_code = keyboard_read_scan_code();
      if(scan_code <= KEYBOARD_MAX_ASCII) {
        c = keyboard_scan_code_to_ascii(scan_code);
      }
    }
    buffer[i] = c;
    if(c == '\n') {
      return (int)(i + 1);
    }
  }
  return (int)i;
}
static int syscall_getpid(struct cpu_state *cpu __attribute__((unused))) {
  return 1;
}

static syscall_handler_t syscall_table[SYSCALL_MAX] = {
  0,
  syscall_write,
  syscall_exit,
  syscall_read,
  syscall_getpid
};

int syscall_entry(struct cpu_state *cpu) {
  if(cpu->eax < SYSCALL_MAX && syscall_table[cpu->eax]) {
    return syscall_table[cpu->eax](cpu);
  }
  return -1;
}
