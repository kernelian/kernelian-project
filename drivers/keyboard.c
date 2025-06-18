#include "vga.h"
#include "keyboard.h"
#include "delay.h"

int ctrl_pressed = 0;
int alt_pressed = 0;

#define KEYBOARD_PORT 0x60
#define KEYBOARD_MAP_SIZE 64

char keyboard_map[KEYBOARD_MAP_SIZE] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' ',
    0,0,0,0,0,0
};

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

    // Modifier handling FIRST
    if (scancode == 0x1D) ctrl_pressed = 1;
    else if (scancode == 0x9D) ctrl_pressed = 0;

    if (scancode == 0x38) alt_pressed = 1;
    else if (scancode == 0xB8) alt_pressed = 0;

    // Then actual key return
    switch (scancode) {
        case 0x11: return 'w';
        case 0x1F: return 's';
        case 0x1E: return 'a';
        case 0x20: return 'd';
        case 0x39: return ' ';
        default: return 0;
    }
}

char get_key_once() {
    static unsigned char last_scancode = 0;
    static int is_repeating = 0;
    static int delay_counter = 0;

    const int repeat_rate_ms = 50;
    const int initial_delay_ms = 300;

    while (1) {
        unsigned char scancode = inb(KEYBOARD_PORT);

        // Key release
        if (scancode & 0x80) {
            if ((scancode & 0x7F) == last_scancode) {
                last_scancode = 0;
                is_repeating = 0;
                delay_counter = 0;
            }
            continue;
        }

        // New key press
        if (scancode != last_scancode) {
            if (scancode == 0x39 || scancode == 0xB9) {
                continue;
            }
            last_scancode = scancode;
            is_repeating = 0;
            delay_counter = 0;

            if (scancode < KEYBOARD_MAP_SIZE) {
                char c = keyboard_map[scancode];
                if (c != 0) return c;
            }
        }

        // Held key logic
        if (scancode == last_scancode) {
            delay_ms(500);
            delay_counter += 10;

            if (!is_repeating && delay_counter >= initial_delay_ms) {
                is_repeating = 1;
                delay_counter = 0;
                if (scancode < KEYBOARD_MAP_SIZE) {
                    char c = keyboard_map[scancode];
                    if (c != 0) return c;
                }
            }

            if (is_repeating && delay_counter >= repeat_rate_ms) {
                delay_counter = 0;
                if (scancode < KEYBOARD_MAP_SIZE) {
                    char c = keyboard_map[scancode];
                    if (c != 0) return c;
                }
            }
        }
    }
}


char get_char_from_keyboard() {
    static unsigned char last_scancode = 0;
    static char last_char = 0;
    static int is_held = 0;
    static uint32_t hold_timer = 0;

    const int initial_delay = 5;  // ms before repeat starts
    const int repeat_rate = 150;     // ms between repeats

    while (1) {
        if (!(inb(0x64) & 0x01)) {
            // If key is being held
            if (is_held) {
                delay_ms(800);
                hold_timer += 10;

                if (hold_timer >= repeat_rate) {
                    hold_timer = 0;
                    return last_char;  // Repeat the last character
                }
            } else {
                delay_ms(1); // Avoid busy-waiting
            }

            continue;
        }

        unsigned char scancode = inb(0x60);

        // Key release
        if (scancode & 0x80) {
            if ((scancode & 0x7F) == last_scancode) {
                is_held = 0;
                last_scancode = 0;
                last_char = 0;
                hold_timer = 0;
            }
            continue;
        }

        // New key press
        if (scancode != last_scancode) {
            last_scancode = scancode;
            hold_timer = 0;
            is_held = 1;

            if (scancode < KEYBOARD_MAP_SIZE) {
                char c = keyboard_map[scancode];
                if (c != 0) {
                    last_char = c;
                    delay_ms(initial_delay); // Wait before repeating
                    return c;
                }
            }
        }
    }
}






char get_key() {
    return get_char_from_keyboard();
}
