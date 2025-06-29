#ifndef PRINT_H
#define PRINT_H

#ifndef NULL
#define NULL ((void*)0)
#endif

void print(const char *text, int *offset);
void print_hex(unsigned int val, int *offset);
void reboot();

#endif // PRINT_H
