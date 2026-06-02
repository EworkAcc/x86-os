#ifndef INCLUDE_GFX_H
#define INCLUDE_GFX_H

#include "../../multiboot.h"

#define GFX_COLOR_BLACK 0x000000
#define GFX_COLOR_WHITE 0xffffff
#define GFX_COLOR_RED 0xff0000
#define GFX_COLOR_GREEN 0x00ff00
#define GFX_COLOR_BLUE 0x0000ff
#define GFX_COLOR_CYAN 0x00ffff
#define GFX_COLOR_GREY 0x808080

struct gfx_state {
  unsigned int available;
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int bpp;
  unsigned char *buffer;
};

void gfx_init(multiboot_info_t *mbinfo);
int gfx_is_available(void);
const struct gfx_state *gfx_get_state(void);
void gfx_draw_pixel(unsigned int x, unsigned int y, unsigned int color);
void gfx_draw_rect(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);
void gfx_draw_char(unsigned int x, unsigned int y, char c, unsigned int foreground, unsigned int background);
void gfx_draw_string(unsigned int x, unsigned int y, const char *text, unsigned int foreground, unsigned int background);
void gfx_demo(void);

#endif
