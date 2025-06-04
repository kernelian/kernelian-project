#!/bin/bash

set -e

echo "[+] Compiling kernel components..."
gcc -m32 -fno-stack-protector -fno-builtin -c kernel.c -o kernel.o
gcc -m32 -fno-stack-protector -fno-builtin -c keyboard.c -o keyboard.o
gcc -m32 -fno-stack-protector -fno-builtin -c vga.c -o vga.o

echo "[+] Assembling bootloader..."
nasm -f elf32 boot.s -o boot.o

echo "[+] Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel boot.o kernel.o vga.o keyboard.o

echo "[+] Creating ISO..."
mv kernel kernelian/boot/kernel
grub-mkrescue -o kernelian.iso kernelian

echo "[+] Launching in QEMU..."
qemu-system-i386 kernelian.iso
