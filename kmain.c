#include "multiboot.h"
#include "drivers/IO/io.h"
#include "drivers/SP/sp.h"
#include "drivers/FB/fb.h"
#include "drivers/INTERRUPTS/pic.h"
#include "drivers/INTERRUPTS/interrupt.h"
#include "drivers/INTERRUPTS/kb.h"
#include "drivers/FILESYSTEM/fs.h"
#include "memory/PAGING/paging.h"
#include "memory/SEG/seg.h"

extern void enter_user_mode(unsigned int entry_point);

int kmain(unsigned int ebx) {
  segments_install_gdt();

  interrupts_install_idt();

  init_paging();
  filesystem_init();

  multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;
  multiboot_module_t* modules = (multiboot_module_t *) mbinfo->mods_addr;
  unsigned int address_of_module = modules->mod_start;
  // unsigned int size = modules->mod_end - modules->mod_start;

  enter_user_mode(address_of_module);
  /*
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
  */
  return 0;
}
