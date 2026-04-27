#include "fs.h"

static struct fs_node fs_nodes[FS_MAX_NODES];
static struct fs_block_device *fs_device = 0;

static unsigned int fs_strlen(const char *value) {
  unsigned int length = 0;
  while(value[length]) {
    length++;
  }
  return length;
}

static int fs_strcmp(const char *a, const char *b) {
  unsigned int i = 0;
  while(a[i] && b[i]) {
    if(a[i] != b[i]) {
      return (int)a[i] - (int)b[i];
    }
    i++;
  }
  return (int)a[i] - (int)b[i];
}

static void fs_memcpy(unsigned char *dest, const unsigned char *src, unsigned int size) {
  unsigned int i;
  for(i = 0; i < size; i++) {
    dest[i] = src[i];
  }
}

static int fs_find_free_node(void) {
  int i;
  for(i = 0; i < FS_MAX_NODES; i++) {
    if(!fs_nodes[i].used) {
      return i;
    }
  }
  return FS_INVALID_NODE;
}

static int fs_find_child(int parent, const char *name) {
  int i;
  for(i = 0; i < FS_MAX_NODES; i++) {
    if(fs_nodes[i].used && fs_nodes[i].parent == parent && fs_strcmp(fs_nodes[i].name, name) == 0) {
      return i;
    }
  }
  return FS_INVALID_NODE;
}

static int fs_copy_name(char *dst, const char *src) {
  unsigned int len = fs_strlen(src);
  unsigned int i;

  if(len == 0 || len >= FS_MAX_NAME_LEN) {
    return -1;
  }

  for(i = 0; i < len; i++) {
    dst[i] = src[i];
  }
  dst[len] = 0;
  return 0;
}

static int vfs_lookup(const char *path) {
  int current = 0;
  unsigned int i = 0;
  char segment[FS_MAX_NAME_LEN];

  if(!path || path[0] != '/') {
    return FS_INVALID_NODE;
  }

  if(path[1] == 0) {
    return 0;
  }

  while(path[i]) {
    unsigned int seg_len = 0;
    if(path[i] == '/') {
      i++;
      continue;
    }

    while(path[i] && path[i] != '/') {
      if(seg_len + 1 >= FS_MAX_NAME_LEN) {
        return FS_INVALID_NODE;
      }
      segment[seg_len++] = path[i++];
    }
    segment[seg_len] = 0;

    current = fs_find_child(current, segment);
    if(current == FS_INVALID_NODE) {
      return FS_INVALID_NODE;
    }
  }

  return current;
}

static int vfs_split_parent(const char *path, int *parent_out, char *name_out) {
  int parent = 0;
  unsigned int i = 0;
  unsigned int last_start = 0;
  unsigned int last_len = 0;
  char segment[FS_MAX_NAME_LEN];

  if(!path || path[0] != '/') {
    return -1;
  }

  if(path[1] == 0) {
    return -1;
  }

  while(path[i]) {
    unsigned int seg_len = 0;
    if(path[i] == '/') {
      i++;
      continue;
    }

    last_start = i;
    while(path[i] && path[i] != '/') {
      i++;
      seg_len++;
    }
    last_len = seg_len;
  }

  if(last_len == 0 || last_len >= FS_MAX_NAME_LEN) {
    return -1;
  }

  i = 1;
  while(path[i]) {
    unsigned int seg_len = 0;
    if(i == last_start) {
      break;
    }
    while(path[i] == '/') {
      i++;
    }
    if(i == last_start || !path[i]) {
      break;
    }
    while(path[i] && path[i] != '/') {
      if(seg_len + 1 >= FS_MAX_NAME_LEN) {
        return -1;
      }
      segment[seg_len++] = path[i++];
    }
    segment[seg_len] = 0;
    parent = fs_find_child(parent, segment);
    if(parent == FS_INVALID_NODE) {
      return -1;
    }
  }

  for(i = 0; i < last_len; i++) {
    name_out[i] = path[last_start + i];
  }
  name_out[last_len] = 0;
  *parent_out = parent;
  return 0;
}

void filesystem_init(void) {
  int i;
  for(i = 0; i < FS_MAX_NODES; i++) {
    fs_nodes[i].used = 0;
  }

  fs_nodes[0].used = 1;
  fs_nodes[0].parent = 0;
  fs_nodes[0].type = FS_NODE_DIRECTORY;
  fs_nodes[0].name[0] = '/';
  fs_nodes[0].name[1] = 0;
  fs_nodes[0].size = 0;
}

void filesystem_set_block_device(struct fs_block_device *device) {
  fs_device = device;
}

int vfs_create_file(const char *path) {
  int parent;
  int index;
  char name[FS_MAX_NAME_LEN];

  if(vfs_lookup(path) != FS_INVALID_NODE) {
    return -1;
  }

  if(vfs_split_parent(path, &parent, name) < 0) {
    return -1;
  }

  if(fs_nodes[parent].type != FS_NODE_DIRECTORY) {
    return -1;
  }

  index = fs_find_free_node();
  if(index == FS_INVALID_NODE) {
    return -1;
  }

  fs_nodes[index].used = 1;
  fs_nodes[index].parent = parent;
  fs_nodes[index].type = FS_NODE_FILE;
  fs_nodes[index].size = 0;
  if(fs_copy_name(fs_nodes[index].name, name) < 0) {
    fs_nodes[index].used = 0;
    return -1;
  }

  return index;
}

int vfs_mkdir(const char *path) {
  int parent;
  int index;
  char name[FS_MAX_NAME_LEN];

  if(vfs_lookup(path) != FS_INVALID_NODE) {
    return -1;
  }

  if(vfs_split_parent(path, &parent, name) < 0) {
    return -1;
  }

  if(fs_nodes[parent].type != FS_NODE_DIRECTORY) {
    return -1;
  }

  index = fs_find_free_node();
  if(index == FS_INVALID_NODE) {
    return -1;
  }

  fs_nodes[index].used = 1;
  fs_nodes[index].parent = parent;
  fs_nodes[index].type = FS_NODE_DIRECTORY;
  fs_nodes[index].size = 0;
  if(fs_copy_name(fs_nodes[index].name, name) < 0) {
    fs_nodes[index].used = 0;
    return -1;
  }

  return index;
}

int vfs_write_file(const char *path, const unsigned char *buffer, unsigned int size) {
  int node = vfs_lookup(path);
  unsigned int write_size = size;

  if(node == FS_INVALID_NODE || fs_nodes[node].type != FS_NODE_FILE) {
    return -1;
  }

  if(write_size > FS_MAX_DATA_SIZE) {
    write_size = FS_MAX_DATA_SIZE;
  }

  fs_memcpy(fs_nodes[node].data, buffer, write_size);
  fs_nodes[node].size = write_size;

  if(fs_device && fs_device->write_block) {
    fs_device->write_block((unsigned int)node, fs_nodes[node].data, write_size);
  }

  return (int)write_size;
}

int vfs_read_file(const char *path, unsigned char *buffer, unsigned int size) {
  int node = vfs_lookup(path);
  unsigned int read_size;

  if(node == FS_INVALID_NODE || fs_nodes[node].type != FS_NODE_FILE) {
    return -1;
  }

  read_size = fs_nodes[node].size;
  if(read_size > size) {
    read_size = size;
  }

  if(fs_device && fs_device->read_block) {
    fs_device->read_block((unsigned int)node, fs_nodes[node].data, fs_nodes[node].size);
  }

  fs_memcpy(buffer, fs_nodes[node].data, read_size);
  return (int)read_size;
}

int vfs_list(const char *path, struct fs_dir_entry *entries, unsigned int max_entries) {
  int node = vfs_lookup(path);
  unsigned int count = 0;
  int i;
  unsigned int j;

  if(node == FS_INVALID_NODE || fs_nodes[node].type != FS_NODE_DIRECTORY) {
    return -1;
  }

  for(i = 0; i < FS_MAX_NODES && count < max_entries; i++) {
    if(!fs_nodes[i].used || fs_nodes[i].parent != node || i == node) {
      continue;
    }

    for(j = 0; j < FS_MAX_NAME_LEN; j++) {
      entries[count].name[j] = fs_nodes[i].name[j];
      if(fs_nodes[i].name[j] == 0) {
        break;
      }
    }

    if(j == FS_MAX_NAME_LEN) {
      entries[count].name[FS_MAX_NAME_LEN - 1] = 0;
    }

    entries[count].type = fs_nodes[i].type;
    count++;
  }

  return (int)count;
}
