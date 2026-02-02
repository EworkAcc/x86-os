#ifndef INCLUDE_TSS_H
#define INCLUDE_TSS_H

struct TSS {
  unsigned int prev_tss;
  unsigned int esp0;
  unsigned int ss0;
  unsigned int esp1;
  unsigned int ss1;
  unsigned int esp2;
  unsigned int ss2;
  unsigned int cr3;
  unsigned int eip;
  unsigned int eflags;
  unsigned int eax;
  unsigned int ecx;
  unsigned int edx;
  unsigned int ebx;
  unsigned int esp;
  unsigned int ebp;
  unsigned int esi;
  unsigned int edi;
  unsigned int es;
  unsigned int cs;
  unsigned int ss;
  unsigned int ds;
  unsigned int fs;
  unsigned int gs;
  unsigned int ldt;
  unsigned short trap;
  unsigned short iomap_base;
} __attribute__((packed));

void tss_init();
void tss_load();

#endif
