#include "io.h"
#include "sp.h"
#include "fb.h"
#include "seg.h"
#include "pic.h"
#include "interrupt.h"
#include "kb.h"

int main() {
  unsigned char scancode, ascii;
  char asciicode[4000];

  segments_install_gdt();

  interrupts_install_idt();
  scancode = keyboard_read_scan_code();
  ascii = keyboard_scan_code_to_ascii(scancode);
  asciicode[0] = ascii;
  serial_write((char *)asciicode, sizeof(asciicode));

  return 0;
}
