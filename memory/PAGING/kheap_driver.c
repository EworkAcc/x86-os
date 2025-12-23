#include "kheap.h"

extern unsigned int KERNEL_PHYSICAL_END;
unsigned int placement_address = (unsigned int)&KERNEL_PHYSICAL_END;

unsigned int kmalloc(unsigned int size) {
  unsigned int mem = placement_address;
  placement_address += size;
  return mem;
}

unsigned int kmalloc_page() {
  if(placement_address & 0xfffff000) {
    placement_address &= 0xfffff000;
    placement_address += 0x1000;
  }
  return kmalloc(0x1000);
}
