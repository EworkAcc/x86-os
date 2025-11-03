#include "io.h"
#include "sp.h"

char *fb = (char *) 0x000b8000;
unsigned int writing_state = 800;
unsigned int cursor_state = 400;
unsigned int mode = 0;

enum BaudRate { Baud_115200=1, Baud_57600, Baud_19200, Baud_9600 };
enum BaudRate divisor = Baud_115200;

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

/*SERIAL PORT DRIVER*/
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
  outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  outb(SERIAL_DATA_PORT(com), ((divisor >> 8) & 0x00ff));
  outb(SERIAL_DATA_PORT(com), divisor & 0x00ff);
}
void serial_configure_line(unsigned short com) {
  outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}
void serial_configure_fifo_buffer(unsigned short com) {
  outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}
void serial_configure_modem(unsigned short com) {
  outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}
int serial_is_transmit_fifo_empty(unsigned int com) {
  return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}
void serial_write_byte(unsigned short port, char byteData) {
  outb(port, byteData);
}
void serial_configure(unsigned short port, unsigned short baudRate) {
  serial_configure_baud_rate(port, baudRate);
  serial_configure_line(port);
  serial_configure_fifo_buffer(port);
  serial_configure_modem(port);
}
int serial_write(unsigned short com, char *buf, unsigned int len) {
  serial_configure(com, divisor);
  unsigned int indexToBuffer = 0;
  while (indexToBuffer < len) {
    if (serial_is_transmit_fifo_empty(com)) {

      serial_write_byte(com, buf[indexToBuffer]);
      indexToBuffer++;
    }
  }
  return 0;
}
