bits 16
org 0x7c00

KERNEL equ 0x1200

section .text
global _start
_start:
	jmp short start
	nop

times 33 db 0

start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x6000

	mov si, msg
	call print

	call read_disk
	
	call delay

	mov si, rdisk
	call print

	mov si, conts
	call print

	mov si, KERNEL
	call print

	cli
	hlt
	jmp $-2

delay:
	push ax
	push cx
	push dx

	mov al, 0
	mov ah, 0x86
	mov cx, 0x001e
	mov dx, 0x8480
	int 0x15

	pop dx
	pop cx
	pop ax

	ret

print:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0e
	int 0x10
	jmp print

.done:
	ret

read_disk:
	xor ax, ax
	mov es, ax
	mov bx, KERNEL
	mov ah, 0x02
	mov al, 0x01
	mov ch, 0x00
	mov cl, 0x02
	mov dh, 0x00
	mov dl, 0x80
	int 0x13
	jc .err
	ret

.err:
	mov si, err
	call print
	cli
	hlt
	jmp $-2

msg db "Hello from Test OS!", 13, 10, 0
err db "Failed to read disk!", 13, 10, 0
rdisk db "Read Disk Successfully", 13, 10, 0
conts db "Contents of second sector: ", 0

times 510 - ($-$$) db 0
dw 0xaa55
