#include "io.h"
#include "fb.h"

char *fb = (char *) 0x000b8000;
unsigned int writing_state = 800;
unsigned int cursor_state = 400;
unsigned int mode = 0;


/*FRAMEBUFFER DRIVER*/
void fb_write_cell(unsigned int i, char c, unsigned char bg, unsigned char fg) {
  fb[i] = c;
  fb[i + 1] = ((bg & 0x0f) << 4) | (fg & 0x0f);
}
void fb_move_cursor(unsigned short pos) {
  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PORT, ((pos >> 8) & 0x00ff));
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PORT, pos & 0x00ff);
}
int fb_write(char *buf, unsigned int len) {
  for (unsigned int i=0; i<len; i++) {
    fb_write_cell((writing_state + (i*2)), *(buf+i), FB_BLACK, FB_WHITE);
  }
  writing_state += len*2;
  cursor_state += len;
  fb_move_cursor(cursor_state);
  return 0;
}
