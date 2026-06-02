#include "display.h"
#include "../FB/fb.h"

static unsigned int display_row = 0;
static unsigned int display_column = 0;
static unsigned char display_foreground = DISPLAY_COLOR_LIGHT_GREY;
static unsigned char display_background = DISPLAY_COLOR_BLACK;

static unsigned int display_cell_offset(unsigned int row, unsigned int column) {
  return ((row * DISPLAY_WIDTH) + column) * 2;
}

static void display_update_cursor(void) {
  fb_move_cursor((unsigned short)((display_row * DISPLAY_WIDTH) + display_column));
}

static void display_blank_cell(unsigned int row, unsigned int column) {
  fb_write_cell(display_cell_offset(row, column), ' ', display_background, display_foreground);
}

static void display_scroll(void) {
  unsigned int row;
  unsigned int column;
  char *video = (char *)0x000b8000;

  for(row = 1; row < DISPLAY_HEIGHT; row++) {
    for(column = 0; column < DISPLAY_WIDTH; column++) {
      unsigned int from = display_cell_offset(row, column);
      unsigned int to = display_cell_offset(row - 1, column);
      video[to] = video[from];
      video[to + 1] = video[from + 1];
    }
  }

  for(column = 0; column < DISPLAY_WIDTH; column++) {
    display_blank_cell(DISPLAY_HEIGHT - 1, column);
  }
}

void display_set_color(unsigned char foreground, unsigned char background) {
  display_foreground = foreground & 0x0f;
  display_background = background & 0x0f;
}

void display_clear(void) {
  unsigned int row;
  unsigned int column;

  for(row = 0; row < DISPLAY_HEIGHT; row++) {
    for(column = 0; column < DISPLAY_WIDTH; column++) {
      display_blank_cell(row, column);
    }
  }

  display_row = 0;
  display_column = 0;
  display_update_cursor();
}

void display_init(void) {
  display_set_color(DISPLAY_COLOR_LIGHT_GREY, DISPLAY_COLOR_BLACK);
  display_clear();
}

void display_newline(void) {
  display_column = 0;
  if(display_row + 1 >= DISPLAY_HEIGHT) {
    display_scroll();
  } else {
    display_row++;
  }
  display_update_cursor();
}

void display_backspace(void) {
  if(display_column > 0) {
    display_column--;
  } else if(display_row > 0) {
    display_row--;
    display_column = DISPLAY_WIDTH - 1;
  } else {
    display_update_cursor();
    return;
  }

  display_blank_cell(display_row, display_column);
  display_update_cursor();
}

void display_putc(char c) {
  if(c == '\n') {
    display_newline();
    return;
  }

  if(c == '\r') {
    display_column = 0;
    display_update_cursor();
    return;
  }

  if(c == '\b') {
    display_backspace();
    return;
  }

  fb_write_cell(display_cell_offset(display_row, display_column), c, display_background, display_foreground);
  display_column++;

  if(display_column >= DISPLAY_WIDTH) {
    display_newline();
  } else {
    display_update_cursor();
  }
}

void display_write(const char *buffer, unsigned int length) {
  unsigned int i;
  for(i = 0; i < length; i++) {
    display_putc(buffer[i]);
  }
}

void display_write_string(const char *string) {
  unsigned int length = 0;
  while(string[length] != 0) length++;
  display_write(string, length);
}
