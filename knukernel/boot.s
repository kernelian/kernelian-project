BITS 32

section .text
align 4
    dd 0x1BADB002           ; Multiboot magic
    dd 0x0                  ; Flags
    dd -(0x1BADB002 + 0x0)  ; Checksum

global start
extern kmain

start:
    cli

    ; Load the GDT
    lgdt [gdt_descriptor]

    ; Set segment registers
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set CS by far jump
    jmp 0x08:flush_cs   ; Code segment selector

flush_cs:
    mov esp, stack_space
    call kmain
    hlt

.halt:
    cli
    hlt
    jmp .halt

section .bss
    resb 8192
stack_space:

section .data
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0

    ; Code segment: base=0, limit=4GB, type=0x9A
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

    ; Data segment: base=0, limit=4GB, type=0x92
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
