#ifndef INCLUDE_FB_H
#define INCLUDE_FB_H

#define FB_COMMAND_PORT 0x3d4
#define FB_DATA_PORT 0x3d5

#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

#define FB_WHITE 15
#define FB_BLACK 0

/*Framebuffer Driver*/
void fb_write_cell(unsigned int i, char c, unsigned char bg, unsigned char fg);
void fb_move_cursor(unsigned short pos);
int fb_write(char *buf, unsigned int len);

#endif
