[bits 32]

global idt_load
extern idtp
extern panic_stub

idt_load:
    lidt [idtp]
    ret

global isr0
isr0:
    cli
    call panic_stub
    hlt

global isr13
isr13:
    cli
    call panic_stub
    hlt

global isr14
isr14:
    cli
    call panic_stub
    hlt
