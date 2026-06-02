#ifndef INCLUDE_WIDGETS_H
#define INCLUDE_WIDGETS_H

struct text_menu {
  const char *title;
  const char **items;
  unsigned int count;
  unsigned int selected;
};

void widget_draw_box(unsigned int row, unsigned int column, unsigned int width, unsigned int height, const char *title);
void widget_draw_menu(unsigned int row, unsigned int column, unsigned int width, const struct text_menu *menu);
void widget_draw_list(unsigned int row, unsigned int column, unsigned int width, const char **items, unsigned int count, unsigned int selected);
void widget_draw_status(const char *left, const char *right);

#endif
