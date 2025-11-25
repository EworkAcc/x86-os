#include "io.h"
#include "sp.h"
#include "fb.h"
#include "seg.h"
#include "interrupt.h"
#include "kb.h"
#include "multiboot.h"

int main(multiboot_info_t *mbinfo) {
  module_t* modules = (module_t*) mbinfo -> mods_addr;
  unsigned int address_of_module = modules -> mod_start;
  
  if((mbinfo -> mods_count) == 1) {
    typdef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t) address_of_module;
    start_program();
  } else return;

  segments_install_gdt();

  interrupts_install_idt();

  # unsigned char scancode, ascii;
  # char asciicode[4000];
  # scancode = keyboard_read_scan_code();
  # ascii = keyboard_scan_code_to_ascii(scancode);
  # asciicode[0] = ascii;
  # serial_write((char *)asciicode, sizeof(asciicode));

  return 0;
}
