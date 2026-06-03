[bits 16]
[org 0x7c00]
global _start
section .text
_start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x9000

	mov si, msg
	call pstr

	call a20_en

	xor bx, bx
	mov ah, 0x02
	mov al, 0x01
	;mov ch, 0x00
	;mov cl, 0x01
	mov dh, 0x00
	mov dl, 0x80
	int 0x13

	mov al, 'Z'
	mov ah, 0x0e
	int 0x10

	cli
	hlt
	jmp $

pstr:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0e
	int 0x10
	jmp pstr

.done:
	ret

a20_en:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

msg db "Hello", 10, 0

times 510 - ($ - $$) db 0
dw 0xaa55
