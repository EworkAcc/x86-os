#include "syscalls.h"
#include "../SP/sp.h"
#include "../INTERRUPTS/kb.h"
#include "../FILESYSTEM/fs.h"

#define USER_MEMORY_LIMIT 0x00400000
#define MAX_OPEN_FILES 16
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_APPEND 0x400
#define O_ACCMODE 0x3
#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR 0x2
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef int (*syscall_handler_t)(struct cpu_state *cpu);
struct syscall_entry { unsigned int number; syscall_handler_t handler; };
struct open_file { int used; int node; unsigned int offset; unsigned int flags; };
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
    if(!open_files[i].used) { open_files[i].used = 1; open_files[i].node = node; open_files[i].offset = 0; open_files[i].flags = 0; return i; }
  }
  return -1;
}

static int fd_is_open(int fd) { return fd >= 3 && fd < MAX_OPEN_FILES && open_files[fd].used; }


static int node_is_open(int node) {
  int i;
  for(i = 3; i < MAX_OPEN_FILES; i++) {
    if(open_files[i].used && open_files[i].node == node) return 1;
  }
  return 0;
}


static int fd_can_read(int fd) {
  unsigned int mode = open_files[fd].flags & O_ACCMODE;
  return mode == O_RDONLY || mode == O_RDWR;
}

static int fd_can_write(int fd) {
  unsigned int mode = open_files[fd].flags & O_ACCMODE;
  return mode == O_WRONLY || mode == O_RDWR;
}


static int dup_fd_from(int oldfd) {
  int newfd = alloc_fd(open_files[oldfd].node);
  if(newfd < 0) return -1;
  open_files[newfd].offset = open_files[oldfd].offset;
  open_files[newfd].flags = open_files[oldfd].flags;
  return newfd;
}


static int dup2_fd(int oldfd, int newfd) {
  if(newfd < 3 || newfd >= MAX_OPEN_FILES) return -1;

  open_files[newfd].used = 1;
  open_files[newfd].node = open_files[oldfd].node;
  open_files[newfd].offset = open_files[oldfd].offset;
  open_files[newfd].flags = open_files[oldfd].flags;
  return newfd;
}


static int syscall_write(struct cpu_state *cpu) {
  unsigned int fd = cpu->ebx;
  if(!user_buffer_is_valid(cpu->ecx, cpu->edx)) return -1;
  if(fd == 1 || fd == 2) { serial_write((char *)cpu->ecx, cpu->edx); return (int)cpu->edx; }
  if(fd_is_open((int)fd)) {
    if(!fd_can_write((int)fd)) return -1;
    if(open_files[fd].flags & O_APPEND) open_files[fd].offset = vfs_node_size(open_files[fd].node);
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
    if(!fd_can_read((int)fd)) return -1;
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
  if((flags & O_ACCMODE) != O_WRONLY && !vfs_node_can_read(node)) return -1;
  if((flags & O_ACCMODE) != O_RDONLY && !vfs_node_can_write(node)) return -1;
  if((flags & (O_TRUNC | O_APPEND)) && !vfs_node_can_write(node)) return -1;
  {
    int fd = alloc_fd(node);
    if(fd < 0) return -1;
    open_files[fd].flags = flags;
    if((open_files[fd].flags & O_ACCMODE) == O_RDONLY && (flags & (O_CREAT | O_TRUNC | O_APPEND))) {
      open_files[fd].flags = (open_files[fd].flags & ~O_ACCMODE) | O_RDWR;
    }
    if(flags & O_TRUNC) {
      if(vfs_truncate_node(node) < 0) return -1;
      open_files[fd].offset = 0;
    }
    if(flags & O_APPEND) {
      open_files[fd].offset = vfs_node_size(node);
    }
    return fd;
  }
}

static int syscall_close(struct cpu_state *cpu) {
  int fd = (int)cpu->ebx;
  if(!fd_is_open(fd)) return -1;
  open_files[fd].used = 0; open_files[fd].node = FS_INVALID_NODE; open_files[fd].offset = 0; open_files[fd].flags = 0;
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
  if(open_files[fd].offset > FS_MAX_DATA_SIZE) open_files[fd].offset = FS_MAX_DATA_SIZE;

  return (int)open_files[fd].offset;
}


static int syscall_dup(struct cpu_state *cpu) {
  int oldfd = (int)cpu->ebx;

  if(!fd_is_open(oldfd)) return -1;
  return dup_fd_from(oldfd);
}


static int syscall_dup2(struct cpu_state *cpu) {
  int oldfd = (int)cpu->ebx;
  int newfd = (int)cpu->ecx;

  if(!fd_is_open(oldfd)) return -1;
  if(newfd < 0) return -1;
  if(oldfd == newfd) return newfd;

  if(fd_is_open(newfd)) {
    open_files[newfd].used = 0;
    open_files[newfd].node = FS_INVALID_NODE;
    open_files[newfd].offset = 0;
    open_files[newfd].flags = 0;
  }

  return dup2_fd(oldfd, newfd);
}


static int syscall_ftruncate(struct cpu_state *cpu) {
  int fd = (int)cpu->ebx;
  unsigned int length = cpu->ecx;

  if(!fd_is_open(fd)) return -1;
  if(!fd_can_write(fd)) return -1;

  if(vfs_resize_node(open_files[fd].node, length) < 0) return -1;
  if(open_files[fd].offset > length) open_files[fd].offset = length;
  return 0;
}


static int syscall_fstat(struct cpu_state *cpu) {
  int fd = (int)cpu->ebx;
  struct fs_stat stat;
  struct fs_stat *user_stat = (struct fs_stat *)cpu->ecx;

  if(!fd_is_open(fd)) return -1;
  if(!user_buffer_is_valid(cpu->ecx, sizeof(struct fs_stat))) return -1;

  if(vfs_stat_node(open_files[fd].node, &stat) < 0) return -1;
  user_stat->type = stat.type;
  user_stat->size = stat.size;
  user_stat->mode = stat.mode;
  user_stat->ctime = stat.ctime;
  user_stat->mtime = stat.mtime;
  return 0;
}

static int syscall_getpid(__attribute__((unused)) struct cpu_state *cpu) { return 1; }
static int syscall_fs_create(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; return vfs_create_file(path); }
static int syscall_fs_write(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; if(!user_buffer_is_valid(cpu->edx, cpu->esi)) return -1; return vfs_write_file(path, (const unsigned char *)cpu->edx, cpu->esi); }
static int syscall_fs_read(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; if(!user_buffer_is_valid(cpu->edx, cpu->esi)) return -1; return vfs_read_file(path, (unsigned char *)cpu->edx, cpu->esi); }
static int syscall_fs_mkdir(struct cpu_state *cpu) { char path[FS_MAX_NAME_LEN]; if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1; return vfs_mkdir(path); }


static int syscall_fs_unlink(struct cpu_state *cpu) {
  int node;
  char path[FS_MAX_NAME_LEN];
  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  node = vfs_node_from_path(path);
  if(node_is_open(node)) return -1;
  return vfs_unlink(path);
}

static int syscall_fs_rmdir(struct cpu_state *cpu) {
  int node;
  char path[FS_MAX_NAME_LEN];
  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  node = vfs_node_from_path(path);
  if(node_is_open(node)) return -1;
  return vfs_rmdir(path);
}


static int syscall_fs_stat(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];
  struct fs_stat stat;
  struct fs_stat *user_stat = (struct fs_stat *)cpu->edx;

  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  if(!user_buffer_is_valid(cpu->edx, sizeof(struct fs_stat))) return -1;
  if(vfs_stat(path, &stat) < 0) return -1;

  user_stat->type = stat.type;
  user_stat->size = stat.size;
  user_stat->mode = stat.mode;
  user_stat->ctime = stat.ctime;
  user_stat->mtime = stat.mtime;
  return 0;
}


static int syscall_fs_rename(struct cpu_state *cpu) {
  char old_path[FS_MAX_NAME_LEN];
  char new_path[FS_MAX_NAME_LEN];

  if(copy_user_string(cpu->ebx, cpu->ecx, old_path, FS_MAX_NAME_LEN) < 0) return -1;
  if(copy_user_string(cpu->edx, cpu->esi, new_path, FS_MAX_NAME_LEN) < 0) return -1;

  return vfs_rename(old_path, new_path);
}


static int syscall_fs_truncate(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];
  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  return vfs_truncate_file(path, cpu->edx);
}


static int syscall_fs_chmod(struct cpu_state *cpu) {
  char path[FS_MAX_NAME_LEN];
  if(copy_user_string(cpu->ebx, cpu->ecx, path, FS_MAX_NAME_LEN) < 0) return -1;
  return vfs_chmod(path, cpu->edx);
}

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
  {SYSCALL_EXIT, syscall_exit}, {SYSCALL_READ, syscall_read}, {SYSCALL_WRITE, syscall_write}, {SYSCALL_OPEN, syscall_open}, {SYSCALL_CLOSE, syscall_close}, {SYSCALL_LSEEK, syscall_lseek}, {SYSCALL_DUP, syscall_dup}, {SYSCALL_DUP2, syscall_dup2}, {SYSCALL_FTRUNCATE, syscall_ftruncate}, {SYSCALL_FSTAT, syscall_fstat},
  {SYSCALL_GETPID, syscall_getpid}, {SYSCALL_FS_CREATE, syscall_fs_create}, {SYSCALL_FS_WRITE, syscall_fs_write}, {SYSCALL_FS_READ, syscall_fs_read},
  {SYSCALL_FS_MKDIR, syscall_fs_mkdir}, {SYSCALL_FS_LIST, syscall_fs_list}, {SYSCALL_FS_UNLINK, syscall_fs_unlink}, {SYSCALL_FS_RMDIR, syscall_fs_rmdir}, {SYSCALL_FS_STAT, syscall_fs_stat}, {SYSCALL_FS_RENAME, syscall_fs_rename}, {SYSCALL_FS_TRUNCATE, syscall_fs_truncate}, {SYSCALL_FS_CHMOD, syscall_fs_chmod}
};

int syscall_dispatch(struct cpu_state *cpu) {
  unsigned int i, count = sizeof(syscall_table) / sizeof(syscall_table[0]);
  for(i = 0; i < count; i++) if(syscall_table[i].number == cpu->eax) return syscall_table[i].handler(cpu);
  return -1;
}
