#ifndef KEYBOARD_H
#define KEYBOARD_H
#define KEYBOARD_PORT 0x60

extern char keyboard_map[];

char get_key(void);
char get_key_once();
char get_char_from_keyboard();

#endif

