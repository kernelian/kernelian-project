// delay.c

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

// Freestanding type definitions
typedef unsigned int  uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

// Freestanding outb (write to I/O port)
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

volatile uint32_t timer_ticks = 0;

void init_pit(uint32_t frequency) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    outb(PIT_COMMAND, 0x36);               // Channel 0, LSB/MSB, mode 3
    outb(PIT_CHANNEL0, divisor & 0xFF);    // Low byte
    outb(PIT_CHANNEL0, (divisor >> 8));    // High byte
}

void pit_interrupt_handler() {
    timer_ticks++;
    outb(0x20, 0x20); // Acknowledge IRQ0 (PIC EOI)
}

uint32_t get_time_ms() {
    return timer_ticks;
}

void sleep_ms(uint32_t ms) {
    uint32_t target = timer_ticks + ms;
    while (timer_ticks < target);
}
