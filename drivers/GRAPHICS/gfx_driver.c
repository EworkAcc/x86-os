#include "gfx.h"
#include "../../memory/PAGING/paging.h"

#define GFX_FONT_WIDTH 8
#define GFX_FONT_HEIGHT 8

static struct gfx_state gfx;

static void gfx_accept_framebuffer(unsigned int width, unsigned int height, unsigned int pitch, unsigned int bpp, unsigned char *buffer) {
  paging_identity_map_range_4mb((unsigned int)buffer, pitch * height);
  gfx.available = 1;
  gfx.unavailable_reason = "available";
  gfx.width = width;
  gfx.height = height;
  gfx.pitch = pitch;
  gfx.bpp = bpp;
  gfx.buffer = buffer;
}

static unsigned int gfx_pack_color(unsigned int color) {
  unsigned int red = (color >> 16) & 0xff;
  unsigned int green = (color >> 8) & 0xff;
  unsigned int blue = color & 0xff;
  return (red << 16) | (green << 8) | blue;
}

static int gfx_supported_bpp(unsigned int bpp) {
  return bpp == 24 || bpp == 32;
}

static int gfx_try_multiboot_framebuffer(multiboot_info_t *mbinfo) {
  if((mbinfo->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) == 0) return 0;

  gfx.unavailable_reason = "multiboot framebuffer is not RGB pixel mode";
  if(mbinfo->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) return 1;

  gfx.unavailable_reason = "multiboot RGB framebuffer is not 24-bit or 32-bit";
  if(!gfx_supported_bpp(mbinfo->framebuffer_bpp)) return 1;

  gfx_accept_framebuffer(
    mbinfo->framebuffer_width,
    mbinfo->framebuffer_height,
    mbinfo->framebuffer_pitch,
    mbinfo->framebuffer_bpp,
    (unsigned char *)(unsigned int)mbinfo->framebuffer_addr
  );
  return 1;
}

static int gfx_try_vbe_framebuffer(multiboot_info_t *mbinfo) {
  multiboot_vbe_mode_info_t *mode_info;

  if((mbinfo->flags & MULTIBOOT_INFO_VBE_INFO) == 0) return 0;
  if(mbinfo->vbe_mode_info == 0) return 0;

  mode_info = (multiboot_vbe_mode_info_t *)mbinfo->vbe_mode_info;

  gfx.unavailable_reason = "VBE mode does not expose a linear framebuffer";
  if((mode_info->mode_attributes & 0x0080) == 0) return 1;

  gfx.unavailable_reason = "VBE framebuffer address is missing";
  if(mode_info->framebuffer_addr == 0) return 1;

  gfx.unavailable_reason = "VBE framebuffer is not 24-bit or 32-bit";
  if(!gfx_supported_bpp(mode_info->bpp)) return 1;

  gfx_accept_framebuffer(
    mode_info->width,
    mode_info->height,
    mode_info->pitch,
    mode_info->bpp,
    (unsigned char *)mode_info->framebuffer_addr
  );
  return 1;
}

void gfx_init(multiboot_info_t *mbinfo) {
  gfx.available = 0;
  gfx.width = 0;
  gfx.height = 0;
  gfx.pitch = 0;
  gfx.bpp = 0;
  gfx.buffer = 0;
  gfx.unavailable_reason = "multiboot info pointer missing";

  if(!mbinfo) return;

  gfx.unavailable_reason = "bootloader did not provide multiboot framebuffer or VBE info";
  if(gfx_try_multiboot_framebuffer(mbinfo)) return;
  if(gfx_try_vbe_framebuffer(mbinfo)) return;
}

int gfx_is_available(void) {
  return gfx.available != 0;
}

const struct gfx_state *gfx_get_state(void) {
  return &gfx;
}

const char *gfx_unavailable_reason(void) {
  return gfx.unavailable_reason;
}

void gfx_draw_pixel(unsigned int x, unsigned int y, unsigned int color) {
  unsigned char *pixel;
  unsigned int packed;

  if(!gfx.available) return;
  if(x >= gfx.width || y >= gfx.height) return;

  pixel = gfx.buffer + (y * gfx.pitch) + (x * (gfx.bpp / 8));
  packed = gfx_pack_color(color);

  pixel[0] = packed & 0xff;
  pixel[1] = (packed >> 8) & 0xff;
  pixel[2] = (packed >> 16) & 0xff;
  if(gfx.bpp == 32) pixel[3] = 0;
}

void gfx_draw_rect(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color) {
  unsigned int row;
  unsigned int column;
  for(row = 0; row < height; row++) {
    for(column = 0; column < width; column++) {
      gfx_draw_pixel(x + column, y + row, color);
    }
  }
}

static unsigned char gfx_glyph_row(char c, unsigned int row) {
  unsigned char seed = (unsigned char)c;
  if(c == ' ') return 0x00;
  if(row == 0 || row == GFX_FONT_HEIGHT - 1) return 0x7e;
  return (unsigned char)(0x42 | (((seed >> (row & 3)) & 1) << 3));
}

void gfx_draw_char(unsigned int x, unsigned int y, char c, unsigned int foreground, unsigned int background) {
  unsigned int row;
  unsigned int column;

  for(row = 0; row < GFX_FONT_HEIGHT; row++) {
    unsigned char bits = gfx_glyph_row(c, row);
    for(column = 0; column < GFX_FONT_WIDTH; column++) {
      unsigned int color = (bits & (0x80 >> column)) ? foreground : background;
      gfx_draw_pixel(x + column, y + row, color);
    }
  }
}

void gfx_draw_string(unsigned int x, unsigned int y, const char *text, unsigned int foreground, unsigned int background) {
  unsigned int i = 0;
  while(text && text[i] != 0) {
    gfx_draw_char(x + (i * GFX_FONT_WIDTH), y, text[i], foreground, background);
    i++;
  }
}

void gfx_demo(void) {
  if(!gfx.available) return;
  gfx_draw_rect(0, 0, gfx.width, gfx.height, GFX_COLOR_BLUE);
  gfx_draw_rect(16, 16, 220, 72, GFX_COLOR_GREY);
  gfx_draw_rect(20, 20, 212, 64, GFX_COLOR_BLACK);
  gfx_draw_string(28, 32, "x86-os graphics", GFX_COLOR_CYAN, GFX_COLOR_BLACK);
  gfx_draw_string(28, 48, "pixel framebuffer ready", GFX_COLOR_WHITE, GFX_COLOR_BLACK);
}
