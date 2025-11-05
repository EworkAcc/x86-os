#include "io.h"
#include "sp.h"
#include "fb.h"

int main() {
  char hello[] = "Hello World!";
  fb_write(hello, 12);
  return 0;
}
