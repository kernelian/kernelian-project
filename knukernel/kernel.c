#include "vga.h"
#include "keyboard.h"
#include "pci.h"
#include "delay.h"
#include "io.h"
extern char get_char_from_keyboard();

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY ((char*)0xB8000)

// Proper scroll logic
void scroll_up() {
    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
        VIDEO_MEMORY[i] = VIDEO_MEMORY[i + SCREEN_WIDTH * 2];
    }

    // Clear the last line
    for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
        VIDEO_MEMORY[i] = ' ';
        VIDEO_MEMORY[i + 1] = 0x07;
    }
}

// Print and detect scroll
unsigned char current_color = WHITE_ON_BLACK;

int print(const char *str, int *offset) {
    int scrolled = 0;

    while (*str) {
        if (*str == '\n') {
            int col = (*offset / 2) % SCREEN_WIDTH;
            *offset += (SCREEN_WIDTH - col) * 2;
        } else {
            if (*offset >= SCREEN_WIDTH * SCREEN_HEIGHT * 2) {
                scroll_up();
                *offset -= SCREEN_WIDTH * 2;
                scrolled = 1;
            }

            set_char_at_video_memory_color(*str, *offset, current_color);
            *offset += 2;
            set_cursor(*offset);
        }
        str++;
    }

    return scrolled;
}





int offset = 0;


// Helper to convert a nibble to hex char
char nibble_to_hex(unsigned char nibble) {
    return (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
}

// Print hex value
void print_hex(unsigned int val, int *offset) {
    print("0x", offset);

    for (int i = 3; i >= 0; i--) {
        unsigned char nibble = (val >> (i * 4)) & 0xF;
        char hex_char[2] = { nibble_to_hex(nibble), '\0' };
        print(hex_char, offset);
    }
}






// RAM command output
void print_ram_info(int *offset) {
    #define TOTAL_RAM_MB 64
    #define USED_RAM_MB 4
    int free_ram = TOTAL_RAM_MB - USED_RAM_MB;

    char msg[64];
    int i = 0;

    const char *prefix = "\nFree RAM: ";
    while (prefix[i]) {
        msg[i] = prefix[i];
        i++;
    }




 














    // Convert free_ram to string
    int start = i;
    int val = free_ram;
    if (val == 0) {
        msg[i++] = '0';
    } else {
        while (val > 0) {
            msg[i++] = '0' + (val % 10);
            val /= 10;
        }

        // Reverse digits
        for (int j = 0; j < (i - start) / 2; j++) {
            char tmp = msg[start + j];
            msg[start + j] = msg[i - 1 - j];
            msg[i - 1 - j] = tmp;
        }
    }

    msg[i++] = ' ';
    msg[i++] = 'M';
    msg[i++] = 'B';
    msg[i++] = '\n';
    msg[i] = '\0';

    print(msg, offset);
}

// Helper function to print a new prompt line cleanly
void new_prompt_line(int *offset) {
    int col = (*offset / 2) % SCREEN_WIDTH;
    *offset += (SCREEN_WIDTH - col) * 2;

    if (*offset >= SCREEN_WIDTH * SCREEN_HEIGHT * 2) {
        scroll_up();
        *offset -= SCREEN_WIDTH * 2;

        // Clear the new last line to prevent double prompt
        for (int i = *offset; i < *offset + SCREEN_WIDTH * 2; i += 2) {
            set_char_at_video_memory(' ', i);
        }
    }

    current_color = 0x2;
    print("Kernelian> ", offset);
    set_cursor(*offset);
}

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY ((char*)0xB8000)

#define true 1
#define false 0

// Clear the screen helper
void clear_screen(int *offset) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        set_char_at_video_memory(' ', i);
    }
    *offset = 0;
}


extern char get_char_from_keyboard();

// The space shooter game function
void space_shooter(int *offset) {
    clear_screen(offset);

    int player_x = SCREEN_WIDTH / 2;
    int player_y = SCREEN_HEIGHT - 2;

    int bullet_x = -1;
    int bullet_y = -1;

    int enemy_x = SCREEN_WIDTH / 2;
    int enemy_y = 1;

    int enemy_alive = true;

    print("Space Shooter! Use 'a'/'d' to move, 'f' to shoot, 'q' to quit.\n", offset);

    while (true) {
        // Draw player
        set_char_at_video_memory('^', (player_y * SCREEN_WIDTH + player_x) * 2);

        // Draw enemy if alive
        if (enemy_alive) {
            set_char_at_video_memory('V', (enemy_y * SCREEN_WIDTH + enemy_x) * 2);
        } else {
            // Clear enemy char if dead
            set_char_at_video_memory(' ', (enemy_y * SCREEN_WIDTH + enemy_x) * 2);
        }

        // Draw bullet
        if (bullet_y >= 0 && bullet_y < SCREEN_HEIGHT) {
            set_char_at_video_memory('|', (bullet_y * SCREEN_WIDTH + bullet_x) * 2);
        }

        char c = get_char_from_keyboard();

        if (c == 'a') {
            // Move left
            if (player_x > 0) {
                set_char_at_video_memory(' ', (player_y * SCREEN_WIDTH + player_x) * 2);
                player_x--;
            }
        } else if (c == 'd') {
            // Move right
            if (player_x < SCREEN_WIDTH - 1) {
                set_char_at_video_memory(' ', (player_y * SCREEN_WIDTH + player_x) * 2);
                player_x++;
            }
        } else if (c == 'f') {
            // Shoot bullet if no active bullet
            if (bullet_y < 0) {
                bullet_x = player_x;
                bullet_y = player_y - 1;
            }
        } else if (c == 'q') {
            clear_screen(offset);
            return;  // quit game, back to shell
        }

        // Move bullet
        if (bullet_y >= 0) {
            // Clear old bullet
            set_char_at_video_memory(' ', (bullet_y * SCREEN_WIDTH + bullet_x) * 2);
            bullet_y--;

            if (bullet_y < 0) {
                // Bullet off screen
                bullet_x = -1;
                bullet_y = -1;
            } else {
                // Check if bullet hit enemy
                if (enemy_alive && bullet_y == enemy_y && bullet_x == enemy_x) {
                    enemy_alive = false;

                    // Clear enemy character
                    set_char_at_video_memory(' ', (enemy_y * SCREEN_WIDTH + enemy_x) * 2);

                    // Clear bullet
                    bullet_x = -1;
                    bullet_y = -1;

                    print("\nEnemy destroyed! Press 'q' to quit.\n", offset);
                }
            }
        }
    }
}






void enter_draw_mode(int* offset_ptr) {
    clear_screen(offset_ptr);

    int x = 0, y = 0;
    int prev_x = 0, prev_y = 0;
    int prev_draw_enabled = -1;
    int prev_cursor_offset = -1;

    char drawn[25][80] = {{0}};
    int draw_enabled = 0;

    print("DRAW MODE: WASD to move, SPACE to draw, Q to quit\n", offset_ptr);

    while (1) {
       
        if ((inb(0x64) & 1)) {
            unsigned char scancode = inb(0x60);

            if (scancode == 0x39) draw_enabled = 1;     // SPACE pressed
            else if (scancode == 0xB9) draw_enabled = 0; // SPACE released

            // Movement and exit
            switch (scancode) {
                case 0x11: y = (y > 0) ? y - 1 : y; break;
                case 0x1F: y = (y < 24) ? y + 1 : y; break;
                case 0x1E: x = (x > 0) ? x - 1 : x; break;
                case 0x20: x = (x < 79) ? x + 1 : x; break;
                case 0x10:
                    clear_screen(offset_ptr);
                    current_color = 0x0B;
                    print("Exited draw mode.\n", offset_ptr);
                    return;
            }
        }

        int prev_offset = (prev_y * 80 + prev_x) * 2;
        int offset = (y * 80 + x) * 2;

        
        if (x != prev_x || y != prev_y || draw_enabled != prev_draw_enabled) {

            // Redraw old position
            if (drawn[prev_y][prev_x]) {
                set_char_at_video_memory_color('O', prev_offset, make_color(15, 0));
            } else {
                set_char_at_video_memory_color(' ', prev_offset, make_color(0, 0));
            }

            // Draw current position
            if (draw_enabled) {
                drawn[y][x] = 1;
                set_char_at_video_memory_color('O', offset, make_color(15, 0));
            } else {
                set_char_at_video_memory_color('_', offset, make_color(7, 0));
            }

            // Move cursor only when needed
            if (offset != prev_cursor_offset) {
                set_cursor(offset);
                prev_cursor_offset = offset;
            }

            // Save state
            prev_x = x;
            prev_y = y;
            prev_draw_enabled = draw_enabled;
        }
    }
}




































// kmain
void kmain(void) {
    current_color = 0x0B;
    int offset = 0;
    print("Hello! Welcome to Kernelian 0.1. Type help for the list of commands.\n", &offset);

    while (1) {
        new_prompt_line(&offset);

        char buffer[128];
        int index = 0;

        while (1) {
            char c = get_char_from_keyboard();

            if (c == '\n' || c == '\r') {
                buffer[index] = '\0';
                break;
            }

            if (c == '\b') {
                if (index > 0) {
                    index--;
                    offset -= 2;
                    set_char_at_video_memory(' ', offset);
                    set_cursor(offset);
                }
            } else if (index < 127) {
                buffer[index++] = c;
                set_char_at_video_memory(c, offset);
                offset += 2;
                set_cursor(offset);
            }
        }

        // Null-terminate just to be sure
        buffer[index] = '\0';

        // === Command Handling ===
        if (strcmp(buffer, "hi") == 0) {
            current_color = 0x0F;
            print("\nHello from Kernelian!", &offset);

        } else if (strcmp(buffer, "kernver") == 0) {
            current_color = 0x0F;
            print("\nKernelian v0.1 (i386) - An OS that says hello to your hardware once again!", &offset);

        } else if (strcmp(buffer, "void") == 0) {
            offset = 0;
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
                set_char_at_video_memory(' ', i);
            }

        } else if (strcmp(buffer, "ram") == 0) {
            current_color = 0x0F;
            print_ram_info(&offset);

        } else if (strcmp(buffer, "why") == 0) {
            current_color = 0x0F;
            print("\nBecause that's how Kernelian rolls.", &offset);

        } else if (strcmp(buffer, "earth") == 0) {
            current_color = 0x0A;
            print("\n           _____\n", &offset);
            print("         .-'.  ':'-.\n", &offset);
            print("       .''::: .:    '.\n", &offset);
            print("      /   :::::'      \\\n", &offset);
            print("     ;.    ':' `       ;\n", &offset);
            print("     |       '..       |\n", &offset);
            print("     ; '      ::::.    ;\n", &offset);
            print("      \\       '::::   /\n", &offset);
            print("       '.      :::  .'\n", &offset);
            print("         '-.___'_.-'\n", &offset);

        } else if (strcmp(buffer, "kernfetch") == 0) {
            current_color = 0x0F;
            print("\nKKK       KKK\n", &offset);
            print("KKK     KKK\n", &offset);
            print("KKK   KKK\n", &offset);
            print("KKK KKK\n", &offset);
            print("KKK KKK\n", &offset);
            print("KKK   KKK\n", &offset);
            print("KKK     KKK\n", &offset);
            print("KKK       KKK\n", &offset);
            print("KKK         KKK\n", &offset);

        } else if (strcmp(buffer, "spaceoshooter") == 0) {
            space_shooter(&offset);

        } else if (strcmp(buffer, "color") == 0) {
            current_color = 0x0C;
            print("\nThis is red text!", &offset);
            current_color = WHITE_ON_BLACK;

        } else if (strcmp(buffer, "draw") == 0) {
            enter_draw_mode(&offset);

        } else if (strcmp(buffer, "pci") == 0) {
            current_color = 0x0E;
            pci_scan(&offset);

        } else if (strncmp(buffer, "echo ", 5) == 0) {
            current_color = 0x0F;
            print("\n", &offset);
            print(&buffer[5], &offset);

        } else if (strcmp(buffer, "help") == 0) {
            current_color = 0x0F;
            print("\nkernver - Output the version of Kernelian\n", &offset);
            print("kernfetch - Output the Kernelian ASCII K logo\n", &offset);
            print("echo <text> - Echo out what you typed\n", &offset);
            print("hi - Output <<Hello From Kernelian!>>\n", &offset);
            print("earth - Output Earth ASCII\n", &offset);
            print("void - Clear the shell\n", &offset);
            print("spaceoshooter - Play a game of Space Shooter\n", &offset);
            print("why - Explains why :-)\n", &offset);
            print("color - Outputs red color\n", &offset);
            print("ram - Output fake RAM usage\n", &offset);
            print("draw - Enter drawing mode\n", &offset);
            print("pci - List PCI buses\n", &offset);

        } else {
            current_color = 0x4;
            print("\nUnknown command.", &offset);
        }
    }
}
