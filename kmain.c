#include "multiboot.h"
#include "io.h"
#include "sp.h"
#include "fb.h"
#include "seg.h"
#include "pic.h"
#include "interrupt.h"
#include "kb.h"

int kmain(unsigned int ebx) {
  segments_install_gdt();

  interrupts_install_idt();

  multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;
  multiboot_module_t* modules = (multiboot_module_t *) mbinfo->mods_addr;
  unsigned int address_of_module = modules->mod_start;

  if((mbinfo->mods_count) == 1) {
    char message[] = "ONE module loaded successfully!";
    serial_write(message, sizeof(message));
    
    typedef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t) address_of_module;
    start_program();

  }else if((mbinfo->mods_count) == 0) {
    char message[] = "Error: No module loaded";
    serial_write(message, sizeof(message));
  }else {
    char message[] = "Error: More than ONE module loaded";
    serial_write(message, sizeof(message));
  }
  /*
  unsigned char scancode, ascii;
  char asciicode[4000];
  scancode = keyboard_read_scan_code();
  ascii = keyboard_scan_code_to_ascii(scancode);
  asciicode[0] = ascii;
  serial_write((char *)asciicode, sizeof(asciicode));
  */ 
  return 0;
}
