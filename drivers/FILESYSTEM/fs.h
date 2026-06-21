#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#define FS_MAX_NODES 32
#define FS_MAX_NAME_LEN 32
#define FS_MAX_DATA_SIZE 512
#define FS_MAX_LIST_ENTRIES 32
#define FS_INVALID_NODE -1

#define FS_MODE_READ 0400
#define FS_MODE_WRITE 0200
#define FS_MODE_EXEC 0100
#define FS_MODE_DEFAULT_FILE 0600
#define FS_MODE_DEFAULT_DIR 0700

enum fs_node_type {
  FS_NODE_FILE = 1,
  FS_NODE_DIRECTORY = 2
};

struct fs_node {
  int used;
  int parent;
  enum fs_node_type type;
  char name[FS_MAX_NAME_LEN];
  unsigned int size;
  unsigned int mode;
  unsigned int ctime;
  unsigned int mtime;
  unsigned char data[FS_MAX_DATA_SIZE];
};

struct fs_block_device {
  int (*read_block)(unsigned int lba, unsigned char *buffer, unsigned int size);
  int (*write_block)(unsigned int lba, const unsigned char *buffer, unsigned int size);
};

struct fs_dir_entry {
  char name[FS_MAX_NAME_LEN];
  enum fs_node_type type;
};

struct fs_stat {
  unsigned int type;
  unsigned int size;
  unsigned int mode;
  unsigned int ctime;
  unsigned int mtime;
};

void filesystem_init(void);
void filesystem_set_block_device(struct fs_block_device *device);

int vfs_create_file(const char *path);
int vfs_mkdir(const char *path);
int vfs_write_file(const char *path, const unsigned char *buffer, unsigned int size);
int vfs_read_file(const char *path, unsigned char *buffer, unsigned int size);
int vfs_list(const char *path, struct fs_dir_entry *entries, unsigned int max_entries);
int vfs_unlink(const char *path);
int vfs_rmdir(const char *path);
int vfs_stat(const char *path, struct fs_stat *stat_out);
int vfs_stat_node(int node, struct fs_stat *stat_out);
int vfs_rename(const char *old_path, const char *new_path);

int vfs_node_from_path(const char *path);
int vfs_is_file(int node);
int vfs_read_node(int node, unsigned int offset, unsigned char *buffer, unsigned int size);
int vfs_write_node(int node, unsigned int offset, const unsigned char *buffer, unsigned int size);
unsigned int vfs_node_size(int node);
int vfs_truncate_node(int node);
int vfs_resize_node(int node, unsigned int new_size);
int vfs_truncate_file(const char *path, unsigned int new_size);
int vfs_chmod(const char *path, unsigned int mode);
int vfs_node_can_read(int node);
int vfs_node_can_write(int node);

#endif
