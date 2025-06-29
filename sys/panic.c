#include "vga.h"
#include "knulib.h"
#include "kernel.h"
#include "keyboard.h"






const char *panic_banner[] = {
    " ____ _____ ___  ____  _ ",
    "/ ___|_   _/ _ \\|  _ \\| |",
    "\\___ \\ | || | | | |_) | |",
    " ___) || || |_| |  __/|_|",
    "|____/ |_| \\___/|_|   (_)",
    NULL
};



void panic_print(const char *msg, int *offset) {
    while (*msg) {
        set_char_at_video_memory_color(*msg++, *offset, make_color(15, 4)); // white on red
        *offset += 2;
    }
    *offset += (80 - ( (*offset / 2) % 80 )) * 2; // go to start of next line
}



// Define a simple VGA clear function using red background
void clear_screen_red() {
    unsigned char *vidmem = (unsigned char *) 0xb8000;
    unsigned char color = make_color(15, 4); // White on red
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vidmem[i] = ' ';
        vidmem[i + 1] = color;
    }
}

__attribute__((noreturn))
void kernel_panic() {
    asm volatile ("cli");

    disable_cursor();

    clear_screen_red();

    // STOP ASCII
    for (int i = 0; panic_banner[i] != NULL; i++) {
        int offset = i * 160;
        panic_print(panic_banner[i], &offset);
    }


    int offset = 7 * 160;
    panic_print("FATAL ERROR: System halted. Something went terribly wrong. This can be an issue deep in the source code of Kernelian.", &offset);
    panic_print("Here is what you can do:", &offset);
    panic_print(" ", &offset);
    panic_print("If you are seeing this kernel panic message for the first time, reboot your", &offset);
    panic_print("computer.", &offset);
    panic_print(" ", &offset);
    panic_print("If you are seeing this kernel panic message for multiple times, this can be a very serious problem in the source code of Kernelian. You can open an issue on ", &offset);
    panic_print("the Kernelian GitHub page.", &offset);
    panic_print(" ", &offset);
    panic_print("Would you like to reboot? [Y/n]", &offset);
    char choice = get_char_from_keyboard();

    if (choice == 'y' || choice == 'Y') {
        reboot();
    } else {
        panic_print("System halted. Reboot manually.", &offset);
        while (1) asm volatile ("hlt");
    }


    while (1) asm volatile ("hlt");
}

