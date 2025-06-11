# Simple KNU Makefile

CC = gcc -m32
AS = nasm
LD = ld -m elf_i386

CFLAGS = -fno-stack-protector -fno-builtin -ffreestanding -Iinclude
LDFLAGS = -T linker.ld

SRC_C = knukernel/kernel.c drivers/vga.c drivers/keyboard.c
OBJ_C = $(SRC_C:.c=.o)

BOOT_OBJ = knukernel/boot.o

ISO_DIR = kernelian
KERNEL_BIN = kernel

.PHONY: all clean run iso

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(BOOT_OBJ) $(OBJ_C)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BOOT_OBJ): knukernel/boot.s
	$(AS) -f elf32 $< -o $@

iso: $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel
	grub-mkrescue -o kernelian.iso $(ISO_DIR)

run: iso
	qemu-system-i386 kernelian.iso

clean:
	rm -f $(KERNEL_BIN) knukernel/*.o drivers/*.o
	rm -f kernelian.iso
