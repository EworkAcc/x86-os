#include "syscalls.h"
#include "../SP/sp.h"
#include "../INTERRUPTS/kb.h"
#include "../FILESYSTEM/fs.h"

#define USER_MEMORY_LIMIT 0x00400000
#define MAX_OPEN_FILES 16
#define O_CREAT 0x40
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef int (*syscall_handler_t)(struct cpu_state *cpu);
struct syscall_entry { unsigned int number; syscall_handler_t handler; };
struct open_file { int used; int node; unsigned int offset; };
static struct open_file open_files[MAX_OPEN_FILES];

static int user_buffer_is_valid(unsigned int address, unsigned int length) {
  unsigned int end;
  if(length == 0) return 1;
  if(address == 0) return 0;
  end = address + length - 1;
  if(end < address) return 0;
  return end < USER_MEMORY_LIMIT;
}

static int user_address_is_valid(unsigned int address) { return address != 0 && address < USER_MEMORY_LIMIT; }

static int copy_user_string(unsigned int address, unsigned int length, char *out, unsigned int out_size) {
  unsigned int i; const char *source = (const char *)address;
  if(length == 0 || length >= out_size) return -1;
  if(!user_buffer_is_valid(address, length)) return -1;
  for(i = 0; i < length; i++) out[i] = source[i];
  out[length] = 0; return 0;
}

static int copy_user_cstring(unsigned int address, char *out, unsigned int out_size) {
  unsigned int i; const char *source;
  if(!user_address_is_valid(address) || out_size == 0) return -1;
  source = (const char *)address;
  for(i = 0; i + 1 < out_size; i++) {
    unsigned int current = address + i;
    if(!user_address_is_valid(current)) return -1;
    out[i] = source[i];
    if(source[i] == 0) return 0;
  }
  out[out_size - 1] = 0;
  return -1;
}

static int alloc_fd(int node) {
  int i;
  for(i = 3; i < MAX_OPEN_FILES; i++) {
    if(!open_files[i].used) { open_files[i].used = 1; open_files[i].node = node; open_files[i].offset = 0; return i; }
  }
  return -1;
}

static int fd_is_open(int fd) { return fd >= 3 && fd < MAX_OPEN_FILES && open_files[fd].used; }

static int syscall_write(struct cpu_state *cpu) {
  unsigned int fd = cpu->ebx;
  if(!user_buffer_is_valid(cpu->ecx, cpu->edx)) return -1;
  if(fd == 1 || fd == 2) { serial_write((char *)cpu->ecx, cpu->edx); return (int)cpu->edx; }
  if(fd_is_open((int)fd)) {
    int written = vfs_write_node(open_files[fd].node, open_files[fd].offset, (const unsigned char *)cpu->ecx, cpu->edx);
    if(written > 0) open_files[fd].offset += (unsigned int)written;
    return written;
  }
  return -1;
}

static int syscall_exit(__attribute__((unused)) struct cpu_state *cpu) { serial_write("user program exit\n", 18); for(;;) asm volatile ("cli; hlt"); return 0; }

static int syscall_read(struct cpu_state *cpu) {
  unsigned int fd = cpu->ebx;
  if(!user_buffer_is_valid(cpu->ecx, cpu->edx)) return -1;
  if(cpu->edx == 0) return 0;
  if(fd == 0) {
    char *buffer = (char *)cpu->ecx; unsigned int i;
    for(i = 0; i < cpu->edx; i++) {
      char c = 0;
      while(c == 0) {
        unsigned char scan_code = keyboard_read_scan_code();
        if(scan_code <= KEYBOARD_MAX_ASCII) c = keyboard_scan_code_to_ascii(scan_code);
      }
      buffer[i] = c;
      if(c == '\n') return (int)(i + 1);
    }
    return (int)i;
  }
  if(fd_is_open((int)fd)) {
    int read = vfs_read_node(open_files[fd].node, open_files[fd].offset, (unsigned char *)cpu->ecx, cpu->edx);
    if(read > 0) open_files[fd].offset += (unsigned int)read;
    return read;
  }
  return -1;
}

static int syscall_open(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN]; unsigned int flags = cpu->ecx; int node;
  if(copy_user_cstring(cpu->ebx, path, FS_MAX_NAME_LEN) < 0) return -1;
  node = vfs_node_from_path(path);
  if(node == FS_INVALID_NODE && (flags & O_CREAT)) node = vfs_create_file(path);
  if(node == FS_INVALID_NODE || !vfs_is_file(node)) return -1;
  return alloc_fd(node);
}

static int syscall_close(struct cpu_state *cpu) {
  int fd = (int)cpu->ebx;
  if(!fd_is_open(fd)) return -1;
  open_files[fd].used = 0; open_files[fd].node = FS_INVALID_NODE; open_files[fd].offset = 0;
  return 0;
}


static int syscall_lseek(struct cpu_state *cpu) {
  int fd = (int)cpu->ebx;
  int offset = (int)cpu->ecx;
  unsigned int whence = cpu->edx;
  unsigned int base;
  unsigned int size;

  if(!fd_is_open(fd)) return -1;

  size = vfs_node_size(open_files[fd].node);

  if(whence == SEEK_SET) {
    if(offset < 0) return -1;
    base = 0;
  } else if(whence == SEEK_CUR) {
    base = open_files[fd].offset;
  } else if(whence == SEEK_END) {
    base = size;
  } else {
    return -1;
  }

  if(offset < 0 && (unsigned int)(-offset) > base) return -1;
  open_files[fd].offset = (unsigned int)((int)base + offset);
  return (int)open_files[fd].offset;
}

static int syscall_getpid(__attribute__((unused)) struct cpu_state *cpu) { return 1; }
static int syscall_fs_create(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; return vfs_create_file(path); }
static int syscall_fs_write(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; if(!user_buffer_is_valid(cpu->edx, cpu->esi)) return -1; return vfs_write_file(path, (const unsigned char *)cpu->edx, cpu->esi); }
static int syscall_fs_read(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; if(!user_buffer_is_valid(cpu->edx, cpu->esi)) return -1; return vfs_read_file(path, (unsigned char *)cpu->edx, cpu->esi); }
static int syscall_fs_mkdir(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; return vfs_mkdir(path); }

static int append_char(char *buffer, unsigned int capacity, unsigned int *index, char value) { if(*index >= capacity) return -1; buffer[*index] = value; (*index)++; return 0; }

static int syscall_fs_list(struct cpu_state *cpu) {
  struct fs_dir_entry entries[FS_MAX_LIST_ENTRIES]; char path[FS_MAX_NAME_LEN]; char *out = (char *)cpu->edx;
  unsigned int out_len = cpu->esi, out_index = 0; int entry_count, i; unsigned int j;
  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  if(!user_buffer_is_valid(cpu->edx, cpu->esi)) return -1;
  entry_count = vfs_list(path, entries, FS_MAX_LIST_ENTRIES);
  if(entry_count < 0) return -1;
  for(i = 0; i < entry_count; i++) {
    for(j = 0; j < FS_MAX_NAME_LEN && entries[i].name[j]; j++) if(append_char(out, out_len, &out_index, entries[i].name[j]) < 0) return (int)out_index;
    if(entries[i].type == FS_NODE_DIRECTORY) if(append_char(out, out_len, &out_index, '/') < 0) return (int)out_index;
    if(append_char(out, out_len, &out_index, '\n') < 0) return (int)out_index;
  }
  return (int)out_index;
}

static struct syscall_entry syscall_table[] = {
  {SYSCALL_EXIT, syscall_exit}, {SYSCALL_READ, syscall_read}, {SYSCALL_WRITE, syscall_write}, {SYSCALL_OPEN, syscall_open}, {SYSCALL_CLOSE, syscall_close}, {SYSCALL_LSEEK, syscall_lseek},
  {SYSCALL_GETPID, syscall_getpid}, {SYSCALL_FS_CREATE, syscall_fs_create}, {SYSCALL_FS_WRITE, syscall_fs_write}, {SYSCALL_FS_READ, syscall_fs_read},
  {SYSCALL_FS_MKDIR, syscall_fs_mkdir}, {SYSCALL_FS_LIST, syscall_fs_list}
};

int syscall_dispatch(struct cpu_state *cpu) {
  unsigned int i, count = sizeof(syscall_table) / sizeof(syscall_table[0]);
  for(i = 0; i < count; i++) if(syscall_table[i].number == cpu->eax) return syscall_table[i].handler(cpu);
  return -1;
}
