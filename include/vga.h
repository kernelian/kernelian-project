#ifndef VGA_H
#define VGA_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f

#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

void set_char_at_video_memory(char character, int offset);
void set_char_at_video_memory_color(char character, int offset, unsigned char color);

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);

void set_cursor(int offset);
int get_cursor();

unsigned char make_color(unsigned char fg, unsigned char bg);

#endif

