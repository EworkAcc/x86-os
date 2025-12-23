#include "seg.h"

static struct GDTDescriptor gdt_descriptors[SEGMENT_DESCRIPTOR_COUNT];

void segments_init_descriptor(int index, unsigned int base_address, unsigned int limit, unsigned char access_byte, unsigned char flags) {
  gdt_descriptors[index].base_low = base_address & 0xffff;
  gdt_descriptors[index].base_middle = (base_address >> 16) & 0xff;
  gdt_descriptors[index].base_high = (base_address >> 24) & 0xff;

  gdt_descriptors[index].limit_low = limit & 0xffff;
  gdt_descriptors[index].limit_and_flags = (limit >> 16) & 0xf;
  gdt_descriptors[index].limit_and_flags |= (flags << 4) & 0xf0;

  gdt_descriptors[index].access_byte = access_byte;
}

void segments_install_gdt() {
  gdt_descriptors[0].base_low = 0;
  gdt_descriptors[0].base_middle = 0;
  gdt_descriptors[0].base_high = 0;
  gdt_descriptors[0].limit_low = 0;
  gdt_descriptors[0].access_byte = 0;
  gdt_descriptors[0].limit_and_flags = 0;

  struct GDT* gdt_ptr = (struct GDT*)gdt_descriptors;
  gdt_ptr->address = (unsigned int)gdt_descriptors;
  gdt_ptr->size = (sizeof(struct GDTDescriptor) * SEGMENT_DESCRIPTOR_COUNT) - 1;

  segments_init_descriptor(1, SEGMENT_BASE, SEGMENT_LIMIT, SEGMENT_CODE_TYPE, SEGMENT_FLAGS_PART);
  segments_init_descriptor(2, SEGMENT_BASE, SEGMENT_LIMIT, SEGMENT_DATA_TYPE, SEGMENT_FLAGS_PART);
  
  segments_load_gdt(*gdt_ptr);
  segments_load_registers();
}
