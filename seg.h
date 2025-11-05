#ifndef INCLUDE_SEG_H
#define INCLUDE_SEG_H

#define SEGMENT_DESCRIPTOR_COUNT 3

#define SEGMENT_BASE 0
#define SEGMENT_LIMIT 0xfffff

#define SEGMENT_CODE_TYPE 0x9a
#define SEGMENT_DATA_TYPE 0x92

#define SEGMENT_FLAGS_PART 0x0c

struct GDT {
  unsigned short size;
  unsigned int address;
} __attribute__((packed));

struct GDTDescriptor {
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access_byte;
  unsigned char limit_and_flags;
  unsigned char base_high;
} __attribute__((packed));

void segments_init_descriptor(int index, unsigned int base_address, unsigned int limit, unsigned char access_byte, unsigned char flags);
void segments_install_gdt();

void segments_load_gdt(struct GDT gdt);
void segments_load_registers();

#endif
