#include "paging.h"
#include "../../drivers/SP/sp.h"

extern void enable_paging(unsigned int*);

unsigned int page_directory[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));
unsigned int page_table[NUM_PAGES * 4] __attribute__((aligned(PAGE_FRAME_SIZE)));

void init_paging() {
  int i;

  for(i = 0; i < NUM_PAGES; ++i) {
    page_directory[i] = 0x00000002;
  }

  for(i = 0; i < NUM_PAGES * 4; ++i) {
    page_table[i] = (i * 0x1000) | 7;
  }

  for(i = 0; i < 4; ++i) {
    page_directory[i] = ((unsigned int)page_table + (i * 0x1000)) | 7;
  }
  enable_paging(page_directory);
}

void paging_identity_map_range_4mb(unsigned int physical_start, unsigned int size) {
  unsigned int start;
  unsigned int end;
  unsigned int index;

  if(size == 0) return;

  start = physical_start & 0xffc00000;
  end = (physical_start + size - 1) & 0xffc00000;

  for(index = start >> 22; index <= (end >> 22) && index < NUM_PAGES; index++) {
    page_directory[index] = (index << 22) | 0x00000087;
  }

  enable_paging(page_directory);
}

void page_fault() {
  char message[] = "Page Fault";
  serial_write(message, sizeof(message));
}
