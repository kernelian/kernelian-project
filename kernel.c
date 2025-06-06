#include "vga.h"
#include "keyboard.h"
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
// Add a global variable to hold the current color (default white on black)
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


// RAM command output
void print_ram_info(int *offset) {
    #define TOTAL_RAM_MB 64
    #define USED_RAM_MB 4
    int free_ram = TOTAL_RAM_MB - USED_RAM_MB;

    char msg[64];
    int i = 0;

    const char *prefix = "Free RAM: ";
    while (prefix[i]) {
        msg[i] = prefix[i];
        i++;
    }




    #define MAX_FILES 16
#define MAX_FILENAME_LENGTH 16
#define FILE_DATA_SIZE 4096  // total bytes for file contents

typedef unsigned char u8;
typedef unsigned int u32;

struct FileEntry {
    char name[MAX_FILENAME_LENGTH];
    u32 start_offset;
    u32 size;
    u8 in_use;
};

struct FileEntry file_table[MAX_FILES];
u8 file_data_area[FILE_DATA_SIZE];

u32 next_free_offset = 0;










void init_file_table() {
    for (u32 i = 0; i < MAX_FILES; i++) {
        file_table[i].in_use = 0;
    }
    next_free_offset = 0;
}





// this doesn't work, probably has to be something with memory corruption.
int create_file(const char* name, const u8* data, u32 size) {
    if (size > FILE_DATA_SIZE - next_free_offset) {
        return -1; // Not enough space
    }

    // Find free slot
    u32 slot = MAX_FILES;
    for (u32 i = 0; i < MAX_FILES; i++) {
        if (file_table[i].in_use == 0) {
            slot = i;
            break;
        }
    }
    if (slot == MAX_FILES) return -2; // No free file slots

    // Copy file data into data area
    for (u32 i = 0; i < size; i++) {
        file_data_area[next_free_offset + i] = data[i];
    }

    // Fill file table entry
    for (u32 i = 0; i < MAX_FILENAME_LENGTH - 1 && name[i] != 0; i++) {
        file_table[slot].name[i] = name[i];
        file_table[slot].name[i+1] = 0;
    }
    file_table[slot].start_offset = next_free_offset;
    file_table[slot].size = size;
    file_table[slot].in_use = 1;

    next_free_offset += size;
    return 0; // Success
}







int read_file(const char* name, u8* out_buffer, u32 max_size) {
    for (u32 i = 0; i < MAX_FILES; i++) {
        if (file_table[i].in_use == 1) {
            // Simple string compare
            int match = 1;
            for (u32 j = 0; j < MAX_FILENAME_LENGTH; j++) {
                if (file_table[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == 0) break;
            }
            if (match) {
                u32 size_to_copy = file_table[i].size;
                if (size_to_copy > max_size) size_to_copy = max_size;
                for (u32 k = 0; k < size_to_copy; k++) {
                    out_buffer[k] = file_data_area[file_table[i].start_offset + k];
                }
                return size_to_copy; // Return number of bytes read
            }
        }
    }
    return -1; // File not found
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





typedef struct {
    int in_use;
    char name[32];
    unsigned char content[256];
    int size;
} File;

File file_table[MAX_FILES];

void init_file_table() {
    for (int i = 0; i < MAX_FILES; i++) {
        file_table[i].in_use = 0;
    }
}

int create_file(const char* name, const unsigned char* content, int size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_table[i].in_use) {
            file_table[i].in_use = 1;
            for (int j = 0; j < 32; j++) {
                file_table[i].name[j] = name[j];
                if (name[j] == '\0') break;
            }
            for (int j = 0; j < size; j++) {
                file_table[i].content[j] = content[j];
            }
            file_table[i].size = size;
            return 0; // success
        }
    }
    return -1; // no space
}

int read_file(const char* name, unsigned char* out_content, int max_size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].in_use) {
            int match = 1;
            for (int j = 0; j < 32; j++) {
                if (file_table[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == '\0') break;
            }
            if (match) {
                int size_to_copy = file_table[i].size < max_size ? file_table[i].size : max_size;
                for (int j = 0; j < size_to_copy; j++) {
                    out_content[j] = file_table[i].content[j];
                }
                return size_to_copy;
            }
        }
    }
    return -1; // file not found
}








void enter_draw_mode(int* offset_ptr) {
    clear_screen(offset_ptr);

    int x = 0, y = 0;
    int prev_x = 0, prev_y = 0;
    char drawn[25][80] = {{0}}; // Tracks where 'O' has been drawn

    
    clear_screen(offset_ptr);
    print("DRAW MODE: WASD to move, SPACE to draw, Q to quit\n", offset_ptr);


    while (1) {
        char key = get_key();

                if (key == 'q') {
            clear_screen(offset_ptr);  // <--- Clear screen before quitting
            current_color = 0x0B;
            print("Exited draw mode.\n", offset_ptr);            
            break;
        }


        // Save previous position
        prev_x = x;
        prev_y = y;

        // Update position
        if (key == 'w' && y > 0) y--;
        if (key == 's' && y < 24) y++;
        if (key == 'a' && x > 0) x--;
        if (key == 'd' && x < 79) x++;

        int prev_offset = (prev_y * 80 + prev_x) * 2;
        int offset = (y * 80 + x) * 2;

        // Erase old cursor
        if (drawn[prev_y][prev_x]) {
            set_char_at_video_memory_color('O', prev_offset, make_color(15, 0)); // Restore drawn char
        } else {
            set_char_at_video_memory_color(' ', prev_offset, make_color(0, 0));  // Just blank space
        }

        // Draw new cursor
        if (key == ' ') {
            drawn[y][x] = 1;
            set_char_at_video_memory_color('O', offset, make_color(15, 0)); // white on black
        } else {
            set_char_at_video_memory_color('_', offset, make_color(7, 0));  // gray underscore
        }

        set_cursor(offset);
    }
}

















// Kernel main function
void kmain(void) {
    current_color = 0x0B;
    int offset = 0;
    print("Hello! Welcome to Kernelian 0.1. Type help for the list of commands.\n", &offset);
    

    init_file_table();
    
    
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

        if (index == 2 && buffer[0] == 'h' && buffer[1] == 'i') {
            current_color = 0x0F;
            print("\nHello from Kernelian!", &offset);

       } else if (index == 7 &&
                   buffer[0] == 'k' && buffer[1] == 'e' && buffer[2] == 'r' &&
                   buffer[3] == 'n' && buffer[4] == 'v' && buffer[5] == 'e' &&
                   buffer[6] == 'r') {
            current_color = 0x0F;
            print("\nKernelian v0.1 (i386) - An OS that says hello to your hardware once again!", &offset);
        

        } else if (index == 4 && buffer[0] == 'v' &&
                   buffer[1] == 'o' && buffer[2] == 'i' && buffer[3] == 'd') {
            offset = 0;
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
                set_char_at_video_memory(' ', i);
            }
        } else if (index == 3 && buffer[0] == 'r' &&
                   buffer[1] == 'a' && buffer[2] == 'm') {
            current_color = 0x0F;
            print_ram_info(&offset);


                   } else if (index == 3 && buffer[0] == 'w' && buffer[1] == 'h' && buffer[2] == 'y') {
                        current_color = 0x0F;
                        print("\nBecause that's how Kernelian rolls.", &offset);

                    } else if (index == 5 &&
           buffer[0] == 'e' && buffer[1] == 'a' &&
           buffer[2] == 'r' && buffer[3] == 't' &&
           buffer[4] == 'h') {
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



    } else if (index == 9 &&
           buffer[0] == 'k' && buffer[1] == 'e' && buffer[2] == 'r' &&
           buffer[3] == 'n' && buffer[4] == 'f' && buffer[5] == 'e' &&
           buffer[6] == 't' && buffer[7] == 'c' && buffer[8] == 'h') {
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


        } else if (index == 13 &&
                   buffer[0] == 's' && buffer[1] == 'p' && buffer[2] == 'a' &&
                   buffer[3] == 'c' && buffer[4] == 'e' && buffer[5] == 'o' &&
                   buffer[6] == 's' && buffer[7] == 'h' && buffer[8] == 'o' &&
                   buffer[9] == 'o' && buffer[10] == 't' && buffer[11] == 'e' &&
                   buffer[12] == 'r') {
            space_shooter(&offset);


} else if (index >= 3 &&
           buffer[0] == 'c' && buffer[1] == 'r' && buffer[2] == 'e' &&
           buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == ' ') {
    // Parse command: create <name> <content>
    int name_start = 7;
    int name_end = name_start;
    while (name_end < index && buffer[name_end] != ' ') name_end++;

    if (name_end >= index - 1) {

        current_color = 0x0F;
        print("\nUsage: create <name> <data>", &offset);
    } else {
        char name[32] = {0};
        unsigned char content[256] = {0};
        int content_len = 0;

        for (int i = name_start; i < name_end && i - name_start < 31; i++) {
            name[i - name_start] = buffer[i];
        }

        int content_start = name_end + 1;
        for (int i = content_start; i < index && content_len < 256; i++) {
            content[content_len++] = (unsigned char)buffer[i];
        }

        int res = create_file(name, content, content_len);
        if (res == 0) {
            current_color = 0x0A;
            print("\nFile created successfully!", &offset);
        } else {
            current_color = 0x4;
            print("\nFailed to create file.", &offset);
        }
    }

} else if (index >= 2 &&
           buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'a' &&
           buffer[3] == 'd' && buffer[4] == ' ') {
    // Parse command: read <name>
    char name[32] = {0};
    int name_len = 0;

    for (int i = 5; i < index && name_len < 31; i++) {
        name[name_len++] = buffer[i];
    }

    unsigned char content[256] = {0};
    int read_bytes = read_file(name, content, 256);
    if (read_bytes > 0) {
        current_color = 0x0F;
        print("\nFile content: ", &offset);
        for (int i = 0; i < read_bytes; i++) {
            char ch[2] = {content[i], 0};
            print(ch, &offset);
            current_color = 0x0F;
        }
    } else {
        current_color = 0x4;
        print("\nFile not found or empty.", &offset);
    }


        } else if (index == 2 && buffer[0] == 'l' && buffer[1] == 's') {
    current_color = 0x0F;
    print("\nFiles:\n", &offset);
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].in_use) {
            print(file_table[i].name, &offset);
            print("\n", &offset);
        }
    }




} else if (index == 5 && buffer[0] == 'c' && buffer[1] == 'o' && buffer[2] == 'l' && buffer[3] == 'o' && buffer[4] == 'r') {
    current_color = 0x0C;  // red color
    print("\nThis is red text!", &offset);
    current_color = WHITE_ON_BLACK; // reset color


} else if (index == 4 && buffer[0] == 'h' && buffer[1] == 'e' && buffer[2] == 'l' && buffer[3] == 'p') {
    current_color = 0x0F;
    print("\nkernver - Output the version of Kernelian\n", &offset);
    print("kernfetch - Output the Kernelian ASCII K logo\n", &offset);
    print("echo <put something in here> - Echo out what you typed out\n", &offset);
    print("ls - List files you created\n", &offset);
    print("read <filename> - Read out the file content\n", &offset);
    // print("create <filename> <file content> - (Not advised, can crash Kernelian!) Create a file with file context, make sure to not add any spaces in the filename, because it would be registered as file content! Use dashes instead.\n", &offset); // Added caution, commented until fixed.  
    print("hi - Output <<Hello From Kernelian!>>\n", &offset);
    print("earth - Output Earth ASCII\n", &offset);
    print("void - Clear the shell\n", &offset);
    print("spaceoshooter - Play a game of Space Shooter\n", &offset);
    print("why - Explains why :-)\n", &offset);
    print("color - Outputs red color\n", &offset);
    print("ram - Output fake RAM usage\n", &offset);
    print("draw - Draw!\n", &offset);




    } else if (index == 4 &&
           buffer[0] == 'd' && buffer[1] == 'r' &&
           buffer[2] == 'a' && buffer[3] == 'w') {
    enter_draw_mode(&offset);






                
    } else if (index >= 5 &&
           buffer[0] == 'e' && buffer[1] == 'c' &&
           buffer[2] == 'h' && buffer[3] == 'o' && buffer[4] == ' ') {
    // Echo everything after "echo "
    current_color = 0x0F; // white, because green would look weird.
    print("\n", &offset);
    print(&buffer[5], &offset);
} else {
    current_color = 0x4;
    print("\nUnknown command.", &offset);
}
    }
}
