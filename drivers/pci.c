#include "kernel.h"
#include "pci.h"

static inline void outl(unsigned short port, unsigned int val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}


unsigned int pci_config_read(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset) {
    unsigned int address;

    address = (1U << 31)               // enable bit
    | ((unsigned int)bus << 16)
    | ((unsigned int)device << 11)
    | ((unsigned int)func << 8)
    | (offset & 0xFC);         // align

    outl(0xCF8, address);
    return inl(0xCFC);
}





extern int offset;



void pci_scan(int *offset) {
    unsigned char bus, device, func;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            for (func = 0; func < 8; func++) {
                unsigned int data = pci_config_read(bus, device, func, 0);
                unsigned short vendor = data & 0xFFFF;

                if (vendor == 0xFFFF) continue;

                unsigned short dev_id = (data >> 16) & 0xFFFF;

                print("\n PCI ", offset);
                print("Bus ", offset);        print_hex(bus, offset);
                print("\n Dev ", offset);       print_hex(device, offset);
                print("\n Func ", offset);      print_hex(func, offset);
                print("\n Vendor: ", offset); print_hex(vendor, offset);
                print("\n Device: ", offset);   print_hex(dev_id, offset);
                print("\n", offset);

                return;
            }
        }
    }
}




