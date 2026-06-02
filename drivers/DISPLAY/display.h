#ifndef INCLUDE_DISPLAY_H
#define INCLUDE_DISPLAY_H

#define DISPLAY_WIDTH 80
#define DISPLAY_HEIGHT 25
#define DISPLAY_TEXT_HEIGHT (DISPLAY_HEIGHT - 1)

#define DISPLAY_COLOR_BLACK 0
#define DISPLAY_COLOR_BLUE 1
#define DISPLAY_COLOR_GREEN 2
#define DISPLAY_COLOR_CYAN 3
#define DISPLAY_COLOR_RED 4
#define DISPLAY_COLOR_MAGENTA 5
#define DISPLAY_COLOR_BROWN 6
#define DISPLAY_COLOR_LIGHT_GREY 7
#define DISPLAY_COLOR_DARK_GREY 8
#define DISPLAY_COLOR_LIGHT_BLUE 9
#define DISPLAY_COLOR_LIGHT_GREEN 10
#define DISPLAY_COLOR_LIGHT_CYAN 11
#define DISPLAY_COLOR_LIGHT_RED 12
#define DISPLAY_COLOR_LIGHT_MAGENTA 13
#define DISPLAY_COLOR_LIGHT_BROWN 14
#define DISPLAY_COLOR_WHITE 15

void display_init(void);
void display_clear(void);
void display_set_color(unsigned char foreground, unsigned char background);
void display_set_cursor(unsigned int row, unsigned int column);
void display_get_cursor(unsigned int *row, unsigned int *column);
void display_putc(char c);
void display_write(const char *buffer, unsigned int length);
void display_write_string(const char *string);
void display_write_at(unsigned int row, unsigned int column, const char *buffer, unsigned int length, unsigned char foreground, unsigned char background);
void display_set_status(const char *status);
void display_newline(void);
void display_backspace(void);

#endif
