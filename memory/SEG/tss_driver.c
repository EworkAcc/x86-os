#include "tss.h"
#include "seg.h"
#include "../PAGING/common.h"

extern unsigned char kernel_stack[];
#define KERNEL_STACK_SIZE 4096

struct TSS tss_entry;

void tss_init() {
  unsigned int tss_size = sizeof(struct TSS);
  unsigned int tss_base = (unsigned int)&tss_entry;

  memset((unsigned char *)&tss_entry, 0, tss_size);

  tss_entry.ss0 = 0x10;
  tss_entry.esp0 = (unsigned int)kernel_stack + KERNEL_STACK_SIZE;
  tss_entry.iomap_base = (unsigned short)tss_size;

  segments_init_descriptor(5, tss_base, tss_size - 1, SEGMENT_TSS_TYPE, 0x00);
}
