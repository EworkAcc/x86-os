#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#define KEYBOARD_MAX_ASCII 83
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_STATUS_OUTPUT_FULL 0x01
#define KEYBOARD_BACKSPACE_SCAN_CODE 0x0e

int keyboard_has_data(void);
unsigned char keyboard_read_scan_code(void);

unsigned char keyboard_scan_code_to_ascii(unsigned char);

#endif
