#include "idt.h"

#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

extern void idt_load();

void set_idt_gate(int n, uint32_t handler) {
    idt[n].base_low = handler & 0xFFFF;
    idt[n].selector = 0x08; // Kernel code segment
    idt[n].zero = 0;
    idt[n].flags = 0x8E; // Present, Ring 0, 32-bit interrupt gate
    idt[n].base_high = (handler >> 16) & 0xFFFF;
}

void idt_init() {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint32_t)&idt;

    // Clear the IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        set_idt_gate(i, 0);
    }

    extern void isr0();
    extern void isr13();
    extern void isr14();

    set_idt_gate(0, (uint32_t)isr0);   // divide-by-zero
    set_idt_gate(13, (uint32_t)isr13); // GPF
    set_idt_gate(14, (uint32_t)isr14); // page fault


    idt_load(); // call assembly to load IDT
}
