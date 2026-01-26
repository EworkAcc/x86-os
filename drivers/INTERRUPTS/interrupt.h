#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

struct IDT {
  unsigned short size;
  unsigned int address;
} __attribute__((packed));

struct IDTDescriptor {
  /*bot 32 bits*/
  unsigned short offset_low; //bit 0-15
  unsigned short segment_selector;
  /*top 32 bits*/
  unsigned char reserved;
  unsigned char type_and_attr;
  unsigned short offset_high; //bit 16-31
} __attribute__((packed));

struct cpu_state {
  unsigned int edi;
  unsigned int esi;
  unsigned int ebp;
  unsigned int edx;
  unsigned int ecx;
  unsigned int ebx;
  unsigned int eax;
} __attribute__((packed));

struct stack_state {
  unsigned int error_code;
  unsigned int eip;
  unsigned int cs;
  unsigned int eflags;
} __attribute__((packed));

void interrupts_install_idt();
void interrupt_handler(struct cpu_state *cpu, unsigned int interrupt, struct stack_state *stack);

void load_idt(unsigned int idt_address);

void interrupt_handler_0(void);
void interrupt_handler_1(void);
void interrupt_handler_2(void);
void interrupt_handler_3(void);
void interrupt_handler_4(void);
void interrupt_handler_5(void);
void interrupt_handler_6(void);
void interrupt_handler_7(void);
void interrupt_handler_8(void);
void interrupt_handler_9(void);
void interrupt_handler_10(void);
void interrupt_handler_11(void);
void interrupt_handler_12(void);
void interrupt_handler_13(void);
void interrupt_handler_14(void);

void interrupt_handler_15(void);

void interrupt_handler_16(void);
void interrupt_handler_17(void);
void interrupt_handler_18(void);
void interrupt_handler_19(void);
void interrupt_handler_20(void);
void interrupt_handler_21(void);

void interrupt_handler_22(void);
void interrupt_handler_23(void);
void interrupt_handler_24(void);
void interrupt_handler_25(void);
void interrupt_handler_26(void);
void interrupt_handler_27(void);

void interrupt_handler_28(void);
void interrupt_handler_29(void);
void interrupt_handler_30(void);

void interrupt_handler_31(void);
void interrupt_handler_32(void);

void interrupt_handler_33(void);

#endif
