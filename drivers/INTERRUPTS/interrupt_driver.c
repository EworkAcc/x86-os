#include "interrupt.h"
#include "pic.h"
#include "kb.h"
#include "../IO/io.h"
#include "../FB/fb.h"
#include "../SP/sp.h"
#include "../../memory/PAGING/paging.h"

static const char *interrupt_names[33] = {
  "#DE divide error",
  "#DB debug exception",
  "NMI interrupt",
  "#BP breakpoint",
  "#OF overflow",
  "#BR bound range exceed",
  "#UD undefined opcode",
  "#NM no math coprocessor (device unavailable)",
  "#DF double fault",
  "#MP segment overrun",
  "#TS invalid TSS",
  "#NP segment not present",
  "#SS stack segment fault",
  "#GP general protection",
  "#PF page fault",
  "15 reserved",
  "#MF floating point error",
  "#AC alignment check",
  "#MC machine check",
  "#XM floating point",
  "#VE virtualization",
  "#CP control protection",
  "22 reserved", "23 reserved", "24 reserved", "25 reserved", "26 reserved", "27 reserved",
  "#HV hypervisor injection",
  "#VC VMM communication",
  "#SX security exception",
  "31 reserved", "32 reserved"
};

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_DE 0
#define INTERRUPTS_DB 1
#define INTERRUPTS_NMI 2
#define INTERRUPTS_BP 3
#define INTERRUPTS_OF 4
#define INTERRUPTS_BR 5
#define INTERRUPTS_UD 6
#define INTERRUPTS_NM 7
#define INTERRUPTS_DF 8
#define INTERRUPTS_MP 9
#define INTERRUPTS_TS 10
#define INTERRUPTS_NP 11
#define INTERRUPTS_SS 12
#define INTERRUPTS_GP 13
#define INTERRUPTS_PF 14

#define INTERRUPTS_MF 16
#define INTERRUPTS_AC 17
#define INTERRUPTS_MC 18
#define INTERRUPTS_XM 19
#define INTERRUPTS_VE 20
#define INTERRUPTS_CP 21

#define INTERRUPTS_HV 28
#define INTERRUPTS_VC 29
#define INTERRUPTS_SX 30

#define INTERRUPTS_KEYBOARD 33

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

void interrupts_init_descriptor(int index, unsigned int address) {
  idt_descriptors[index].offset_high = (address >> 16) & 0xffff;
  idt_descriptors[index].offset_low = (address & 0xffff);

  idt_descriptors[index].segment_selector = 0x08;
  idt_descriptors[index].reserved = 0x00;

  idt_descriptors[index].type_and_attr = (0x01 << 7) | (0x00 << 6) | (0x00 << 5) | 0xe;
}

void interrupts_install_idt() {
  interrupts_init_descriptor(INTERRUPTS_DE, (unsigned int) interrupt_handler_0);
  interrupts_init_descriptor(INTERRUPTS_DB, (unsigned int) interrupt_handler_1);
  interrupts_init_descriptor(INTERRUPTS_NMI, (unsigned int) interrupt_handler_2);
  interrupts_init_descriptor(INTERRUPTS_BP, (unsigned int) interrupt_handler_3);
  interrupts_init_descriptor(INTERRUPTS_OF, (unsigned int) interrupt_handler_4);
  interrupts_init_descriptor(INTERRUPTS_BR, (unsigned int) interrupt_handler_5);
  interrupts_init_descriptor(INTERRUPTS_UD, (unsigned int) interrupt_handler_6);
  interrupts_init_descriptor(INTERRUPTS_NM, (unsigned int) interrupt_handler_7);
  interrupts_init_descriptor(INTERRUPTS_DF, (unsigned int) interrupt_handler_8);
  interrupts_init_descriptor(INTERRUPTS_MP, (unsigned int) interrupt_handler_9);
  interrupts_init_descriptor(INTERRUPTS_TS, (unsigned int) interrupt_handler_10);
  interrupts_init_descriptor(INTERRUPTS_NP, (unsigned int) interrupt_handler_11);
  interrupts_init_descriptor(INTERRUPTS_SS, (unsigned int) interrupt_handler_12);
  interrupts_init_descriptor(INTERRUPTS_GP, (unsigned int) interrupt_handler_13);
  interrupts_init_descriptor(INTERRUPTS_PF, (unsigned int) interrupt_handler_14);
  
  interrupts_init_descriptor(15, (unsigned int) interrupt_handler_15);

  interrupts_init_descriptor(INTERRUPTS_MF, (unsigned int) interrupt_handler_16);
  interrupts_init_descriptor(INTERRUPTS_AC, (unsigned int) interrupt_handler_17);
  interrupts_init_descriptor(INTERRUPTS_MC, (unsigned int) interrupt_handler_18);
  interrupts_init_descriptor(INTERRUPTS_XM, (unsigned int) interrupt_handler_19);
  interrupts_init_descriptor(INTERRUPTS_VE, (unsigned int) interrupt_handler_20);
  interrupts_init_descriptor(INTERRUPTS_CP, (unsigned int) interrupt_handler_21);

  interrupts_init_descriptor(22, (unsigned int) interrupt_handler_22);
  interrupts_init_descriptor(23, (unsigned int) interrupt_handler_23);
  interrupts_init_descriptor(24, (unsigned int) interrupt_handler_24);
  interrupts_init_descriptor(25, (unsigned int) interrupt_handler_25);
  interrupts_init_descriptor(26, (unsigned int) interrupt_handler_26);
  interrupts_init_descriptor(27, (unsigned int) interrupt_handler_27);

  interrupts_init_descriptor(INTERRUPTS_HV, (unsigned int) interrupt_handler_28);
  interrupts_init_descriptor(INTERRUPTS_VC, (unsigned int) interrupt_handler_29);
  interrupts_init_descriptor(INTERRUPTS_SX, (unsigned int) interrupt_handler_30);

  interrupts_init_descriptor(31, (unsigned int) interrupt_handler_31);
  interrupts_init_descriptor(32, (unsigned int) interrupt_handler_32);

  interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (unsigned int) interrupt_handler_33);

  idt.address = (int) &idt_descriptors;
  idt.size = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT - 1;
  
  load_idt((int) &idt);

  pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);
}

static void serial_write_hex(unsigned int value) {
  char hex[9];
  const char *digits = "0123456789abcdef";
  for(int i = 7; i >= 0; --i) {
    hex[i] = digits[value & 0xf];
    value >>= 4;
  }
  hex[8] = 0;
  serial_write(hex, 8);
}

void interrupt_handler(__attribute__((unused)) struct cpu_state cpu, unsigned int interrupt, __attribute__((unused)) struct stack_state stack) {
  if(interrupt == INTERRUPTS_KEYBOARD) {
    unsigned char scan_code = keyboard_read_scan_code();
    if(scan_code <= KEYBOARD_MAX_ASCII) {
      char c = keyboard_scan_code_to_ascii(scan_code);
      serial_write(&c, 1);
    }
    pic_acknowledge(interrupt);
    return;
  }

  if(interrupt == INTERRUPTS_PF) {
    page_fault();
    return;
  }

  serial_write("cpu exception", 13);

  if(interrupt < 33) {
    const char *name = interrupt_names[interrupt];
    int len = 0;
    while(name[len]) len++;
    serial_write((char*)name, len);
  }

  serial_write("\nINT: 0x", 8);
  serial_write_hex(interrupt);
  
  serial_write("\nEIP: 0x", 9);
  serial_write_hex(stack.eip);

  serial_write("\nCS: 0x", 9);
  serial_write_hex(stack.cs);

  serial_write("\nEFLAGS: 0x", 12);
  serial_write_hex(stack.eflags);

  serial_write("\nERROR CODE: 0x", 16);
  serial_write_hex(stack.error_code);

  serial_write("\nEAX: 0x", 9); serial_write_hex(cpu.eax);
  serial_write("\nEBX: 0x", 9); serial_write_hex(cpu.ebx);
  serial_write("\nECX: 0x", 9); serial_write_hex(cpu.ecx);
  serial_write("\nEDX: 0x", 9); serial_write_hex(cpu.edx);
  serial_write("\nEBP: 0x", 9); serial_write_hex(cpu.ebp);
  serial_write("\nESI: 0x", 9); serial_write_hex(cpu.esi);
  serial_write("\nEDI: 0x", 9); serial_write_hex(cpu.edi);

  serial_write("\nSYSTEM HALTED\n", 16);

  for (;;) {
    asm volatile ("cli; hlt");
  }
}
