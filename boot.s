bits 16
org 0x7c00

section .text
global _start

_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov si, msg
    call print_str

    cli
    hlt
    jmp $

print_str:
    lodsb
    test al, al
    jz .done
    
    mov ah, 0x0e
    mov bh, 0x00
    int 0x10

    jmp print_str

.done:
    ret


msg:
    db "Welcome to Bikt OS, Loading Kernel...", 10, 0
    
times 510 - ($ - $$) db 0
dw 0xaa55