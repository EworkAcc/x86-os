#include "widgets.h"
#include "../DISPLAY/display.h"

static unsigned int widget_strlen(const char *value) {
  unsigned int length = 0;
  if(!value) return 0;
  while(value[length] != 0) length++;
  return length;
}

static void widget_putc_at(unsigned int row, unsigned int column, char c, unsigned char foreground, unsigned char background) {
  display_write_at(row, column, &c, 1, foreground, background);
}

static void widget_fill(unsigned int row, unsigned int column, unsigned int width, char c, unsigned char foreground, unsigned char background) {
  unsigned int i;
  for(i = 0; i < width; i++) widget_putc_at(row, column + i, c, foreground, background);
}

static void widget_write_clipped(unsigned int row, unsigned int column, unsigned int width, const char *text, unsigned char foreground, unsigned char background) {
  unsigned int length = widget_strlen(text);
  if(length > width) length = width;
  display_write_at(row, column, text, length, foreground, background);
}

void widget_draw_box(unsigned int row, unsigned int column, unsigned int width, unsigned int height, const char *title) {
  unsigned int y;
  unsigned int inner_width;

  if(width < 2 || height < 2) return;
  inner_width = width - 2;

  widget_putc_at(row, column, '+', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
  widget_fill(row, column + 1, inner_width, '-', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
  widget_putc_at(row, column + width - 1, '+', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);

  for(y = 1; y + 1 < height; y++) {
    widget_putc_at(row + y, column, '|', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
    widget_fill(row + y, column + 1, inner_width, ' ', DISPLAY_COLOR_LIGHT_GREY, DISPLAY_COLOR_BLACK);
    widget_putc_at(row + y, column + width - 1, '|', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
  }

  widget_putc_at(row + height - 1, column, '+', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
  widget_fill(row + height - 1, column + 1, inner_width, '-', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);
  widget_putc_at(row + height - 1, column + width - 1, '+', DISPLAY_COLOR_LIGHT_CYAN, DISPLAY_COLOR_BLACK);

  if(title && title[0] != 0 && width > 4) {
    widget_write_clipped(row, column + 2, width - 4, title, DISPLAY_COLOR_LIGHT_BROWN, DISPLAY_COLOR_BLACK);
  }
}

void widget_draw_list(unsigned int row, unsigned int column, unsigned int width, const char **items, unsigned int count, unsigned int selected) {
  unsigned int i;
  if(width < 4 || !items) return;

  for(i = 0; i < count; i++) {
    unsigned char foreground = DISPLAY_COLOR_LIGHT_GREY;
    unsigned char background = DISPLAY_COLOR_BLACK;
    char marker = ' ';

    if(i == selected) {
      foreground = DISPLAY_COLOR_BLACK;
      background = DISPLAY_COLOR_LIGHT_GREY;
      marker = '>';
    }

    widget_fill(row + i, column, width, ' ', foreground, background);
    widget_putc_at(row + i, column + 1, marker, foreground, background);
    widget_write_clipped(row + i, column + 3, width - 4, items[i], foreground, background);
  }
}

void widget_draw_menu(unsigned int row, unsigned int column, unsigned int width, const struct text_menu *menu) {
  unsigned int height;
  if(!menu) return;
  height = menu->count + 4;
  widget_draw_box(row, column, width, height, menu->title);
  widget_draw_list(row + 2, column + 1, width - 2, menu->items, menu->count, menu->selected);
}

void widget_draw_status(const char *left, const char *right) {
  unsigned int left_len = widget_strlen(left);
  unsigned int right_len = widget_strlen(right);
  unsigned int right_column = 0;
  char line[DISPLAY_WIDTH + 1];
  unsigned int i;

  for(i = 0; i < DISPLAY_WIDTH; i++) line[i] = ' ';
  line[DISPLAY_WIDTH] = 0;

  if(left_len > DISPLAY_WIDTH) left_len = DISPLAY_WIDTH;
  for(i = 0; i < left_len; i++) line[i] = left[i];

  if(right_len > DISPLAY_WIDTH) right_len = DISPLAY_WIDTH;
  right_column = DISPLAY_WIDTH - right_len;
  for(i = 0; i < right_len; i++) line[right_column + i] = right[i];

  display_set_status(line);
}
