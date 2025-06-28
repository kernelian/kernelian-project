#include <stdint.h>
#include "vga.h"
#include "keyboard.h"

#define KEYBOARD_PORT 0x60

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

char keyboard_map[] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' ',
};

char keyboard_shift_map[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','\"','~',0,
    '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,' ',
};

char get_char_from_keyboard() {
    static uint8_t last_key_pressed = 0;
    static int shift_pressed = 0;
    static int caps_lock_on = 0;
    static int caps_lock_down = 0;

    while (1) {
        uint8_t scancode = inb(KEYBOARD_PORT);

        // Key release
        if (scancode & 0x80) {
            uint8_t released = scancode & 0x7F;

            if (released == 0x2A || released == 0x36) {
                shift_pressed = 0;
            } else if (released == 0x3A) {
                caps_lock_down = 0; // Caps Lock key released
            }

            if (released == last_key_pressed) {
                last_key_pressed = 0;
            }

            continue;
        }

        // Shift press
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            continue;
        }

        // Caps Lock press (toggle only on new press)
        if (scancode == 0x3A) {
            if (!caps_lock_down) {
                caps_lock_on = !caps_lock_on;
                caps_lock_down = 1;
            }
            continue;
        }

        if (scancode == last_key_pressed) {
            continue;
        }

        last_key_pressed = scancode;

        if (scancode < sizeof(keyboard_map)) {
            char c = keyboard_map[scancode];

            // Letters affected by caps/shift logic
            if (c >= 'a' && c <= 'z') {
                int upper = shift_pressed ^ caps_lock_on;
                return upper ? (c - 32) : c;
            }

            // Non-letters only affected by shift
            if (shift_pressed && keyboard_shift_map[scancode]) {
                return keyboard_shift_map[scancode];
            }

            return c;
        }
    }
}

char get_key() {
    return get_char_from_keyboard();
}
