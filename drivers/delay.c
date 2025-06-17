#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

void init_pit(uint32_t frequency) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    outb(PIT_COMMAND, 0x36);               // Channel 0, LSB/MSB, mode 3
    outb(PIT_CHANNEL0, divisor & 0xFF);    // Low byte
    outb(PIT_CHANNEL0, (divisor >> 8));    // High byte
}

volatile uint32_t timer_ticks = 0;

void pit_interrupt_handler() {
    timer_ticks++;
    // Acknowledge interrupt if needed
}

uint32_t get_time_ms() {
    return timer_ticks;
}

void sleep_ms(uint32_t ms) {
    uint32_t target = timer_ticks + ms;
    while (timer_ticks < target);
}

