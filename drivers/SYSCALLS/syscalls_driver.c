#include "syscalls.h"
#include "../SP/sp.h"
#include "../INTERRUPTS/kb.h"
#include "../FILESYSTEM/fs.h"

#define USER_MEMORY_LIMIT 0x00400000

typedef int (*syscall_handler_t)(struct cpu_state *cpu);
struct syscall_entry {
  unsigned int number;
  syscall_handler_t handler;
};

static int user_buffer_is_valid(unsigned int address, unsigned int length) {
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

static int copy_user_string(unsigned int address, unsigned int length, char *out, unsigned int out_size) {
  unsigned int i;
  const char *source = (const char *)address;

  if(length == 0 || length >= out_size) {
    return -1;
  }

  if(!user_buffer_is_valid(address, length)) {
    return -1;
  }

  for(i = 0; i < length; i++) {
    out[i] = source[i];
  }
  out[length] = 0;
  return 0;
}

static int syscall_write(struct cpu_state *cpu) {
  if(!user_buffer_is_valid(cpu->ebx, cpu->ecx)) {
    return -1;
  }

  serial_write((char *)cpu->ebx, cpu->ecx);
  return (int)cpu->ecx;
}

static int syscall_exit(__attribute__((unused)) struct cpu_state *cpu) {
  serial_write("user program exit\n", 18);
  for(;;) {
    asm volatile ("cli; hlt");
  }
  return 0;
}

static int syscall_read(struct cpu_state *cpu) {
  if(!user_buffer_is_valid(cpu->ebx, cpu->ecx)) {
    return -1;
  }

  if(cpu->ecx == 0) {
    return 0;
  }

  char *buffer = (char *)cpu->ebx;
  unsigned int i;
  for(i = 0; i < cpu->ecx; i++) {
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

static int syscall_getpid(__attribute__((unused)) struct cpu_state *cpu) {
  return 1;
}

static int syscall_fs_create(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) {
    return -1;
  }

  return vfs_create_file(path);
}

static int syscall_fs_write(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) {
    return -1;
  }

  if(!user_buffer_is_valid(cpu->edx, cpu->esi)) {
    return -1;
  }

  return vfs_write_file(path, (const unsigned char *)cpu->edx, cpu->esi);
}

static int syscall_fs_read(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) {
    return -1;
  }

  if(!user_buffer_is_valid(cpu->edx, cpu->esi)) {
    return -1;
  }

  return vfs_read_file(path, (unsigned char *)cpu->edx, cpu->esi);
}

static int syscall_fs_mkdir(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) {
    return -1;
  }

  return vfs_mkdir(path);
}

static int append_char(char *buffer, unsigned int capacity, unsigned int *index, char value) {
  if(*index >= capacity) {
    return -1;
  }
  buffer[*index] = value;
  (*index)++;
  return 0;
}

static int syscall_fs_list(struct cpu_state *cpu) {
  struct fs_dir_entry entries[FS_MAX_LIST_ENTRIES];
  char path[FS_MAX_NAME_LEN];
  char *out = (char *)cpu->edx;
  unsigned int out_len = cpu->esi;
  unsigned int out_index = 0;
  int entry_count;
  int i;
  unsigned int j;

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) {
    return -1;
  }

  if(!user_buffer_is_valid(cpu->edx, cpu->esi)) {
    return -1;
  }

  entry_count = vfs_list(path, entries, FS_MAX_LIST_ENTRIES);
  if(entry_count < 0) {
    return -1;
  }

  for(i = 0; i < entry_count; i++) {
    for(j = 0; j < FS_MAX_NAME_LEN && entries[i].name[j]; j++) {
      if(append_char(out, out_len, &out_index, entries[i].name[j]) < 0) {
        return (int)out_index;
      }
    }

    if(entries[i].type == FS_NODE_DIRECTORY) {
      if(append_char(out, out_len, &out_index, '/') < 0) {
        return (int)out_index;
      }
    }

    if(append_char(out, out_len, &out_index, '\n') < 0) {
      return (int)out_index;
    }
  }

  return (int)out_index;
}

static struct syscall_entry syscall_table[] = {
  {SYSCALL_EXIT, syscall_exit},
  {SYSCALL_READ, syscall_read},
  {SYSCALL_WRITE, syscall_write},
  {SYSCALL_GETPID, syscall_getpid},
  {SYSCALL_FS_CREATE, syscall_fs_create},
  {SYSCALL_FS_WRITE, syscall_fs_write},
  {SYSCALL_FS_READ, syscall_fs_read},
  {SYSCALL_FS_MKDIR, syscall_fs_mkdir},
  {SYSCALL_FS_LIST, syscall_fs_list}
};

int syscall_dispatch(struct cpu_state *cpu) {
  unsigned int i;
  unsigned int count = sizeof(syscall_table) / sizeof(syscall_table[0]);

  for(i = 0; i < count; i++) {
    if(syscall_table[i].number == cpu->eax) {
      return syscall_table[i].handler(cpu);
    }
  }

  return -1;
}
