#include "paging.h"
#include "../../drivers/SP/sp.h"

extern void enable_paging(unsigned int*);

unsigned int page_directory[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));
unsigned int page_table[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));

void init_paging() {
  int i;

  for(i = 0; i < NUM_PAGES; i++) {
    page_directory[i] = 0x00000002;
  }

  for(i = 0; i < NUM_PAGES; i++) {
    page_table[i] = (i * 0x1000) | 3;
  }

  page_directory[0] = ((unsigned int)page_table) | 3;
  enable_paging(page_directory);
}

void page_fault() {
  char message[] = "Page Fault";
  serial_write(message, sizeof(message));
}
