#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#define FS_MAX_NODES 32
#define FS_MAX_NAME_LEN 32
#define FS_MAX_DATA_SIZE 512
#define FS_MAX_LIST_ENTRIES 32
#define FS_INVALID_NODE -1

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

void filesystem_init(void);
void filesystem_set_block_device(struct fs_block_device *device);

int vfs_create_file(const char *path);
int vfs_mkdir(const char *path);
int vfs_write_file(const char *path, const unsigned char *buffer, unsigned int size);
int vfs_read_file(const char *path, unsigned char *buffer, unsigned int size);
int vfs_list(const char *path, struct fs_dir_entry *entries, unsigned int max_entries);

int vfs_node_from_path(const char *path);
int vfs_is_file(int node);
int vfs_read_node(int node, unsigned int offset, unsigned char *buffer, unsigned int size);
int vfs_write_node(int node, unsigned int offset, const unsigned char *buffer, unsigned int size);
unsigned int vfs_node_size(int node);

#endif
