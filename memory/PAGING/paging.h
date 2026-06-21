#ifndef PAGING_H
#define PAGING_H

#define NUM_PAGES 1024
#define PAGE_FRAME_SIZE 4096

#define PRESENT 1
#define PAGE_READONLY 0
#define PAGE_READWRITE 1
#define PAGE_USER 1
#define PAGE_KERNEL 0
#define PAGE_SIZE_4KB 0
#define PAGE_SIZE4MB 1

typedef struct page {
  unsigned int present : 1;
  unsigned int rw : 1;
  unsigned int user : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int unused : 7;
  unsigned int frame : 20;
} page_t;

typedef struct page_table {
  page_t pages[1024] __attribute__((aligned(4096)));
} page_table_t;

typedef struct page_directory {
  page_table_t *tables[1024];
  
  unsigned int tablesPhysical[1024];

  unsigned int physicalAddr;
} page_directory_t;

void init_paging();
void paging_identity_map_range_4mb(unsigned int physical_start, unsigned int size);
void page_fault();

#endif
