#include "common.h"

void memset(unsigned char *dest, unsigned char val, unsigned int len) {
  unsigned char *temp = (unsigned char *)dest;
  for( ; len != 0; len--) *temp++ = val;
}
