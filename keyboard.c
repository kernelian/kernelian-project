#include <stdint.h>
#include "vga.h"
#include "keyboard.h"

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Simple polling function
char poll_keyboard() {
    if ((inb(0x64) & 1) == 0) {
        return 0; // No key pressed
    }

    unsigned char scancode = inb(0x60);

     switch (scancode) {
        case 0x11: return 'w'; // W key
        case 0x1F: return 's'; // S key
        case 0x1E: return 'a'; // A key
        case 0x20: return 'd'; // D key
        case 0x39: return ' '; // Space key
        default: return 0; // Unsupported key
    }
}

#define KEYBOARD_PORT 0x60

char keyboard_map[] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' ', // space
    // Remaining keys ignored for now
};

char get_char_from_keyboard() {
    static uint8_t last_key_pressed = 0; // track last pressed key (not released)
    while (1) {
        uint8_t scancode = port_byte_in(KEYBOARD_PORT);

        // Check if key release (bit 7 set)
        if (scancode & 0x80) {
            if ((scancode & 0x7F) == last_key_pressed) {
                last_key_pressed = 0;
            }
            continue;
        }

        if (scancode == last_key_pressed) {
            continue;
        }

        last_key_pressed = scancode;

        if (scancode < sizeof(keyboard_map)) {
            char c = keyboard_map[scancode];
            if (c != 0) return c;
        }
    }
}

char get_key() {
    return get_char_from_keyboard(); // or poll_keyboard();
}
