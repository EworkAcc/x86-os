#include "io.h"
#include "sp.h"
#include "fb.h"
#include "seg.h"

int main() {
  char hello[] = "Hello World!";

  fb_write(hello, 12);
  serial_write(0x3F8, hello, 12);
  segments_install_gdt();

  return 0;
}
