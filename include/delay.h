#ifndef DELAY_H
#define DELAY_H

// Define your own fixed-width types
typedef unsigned int uint32_t;

extern volatile uint32_t timer_ticks;

void init_pit(uint32_t frequency);

static inline void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; ++i) {
        for (volatile uint32_t j = 0; j < 10000; ++j) {
            __asm__ __volatile__("nop");
        }
    }
}

#endif
