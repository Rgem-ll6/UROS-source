[bits 16]
[org 0x7c00]

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
	mov sp, 0x7000

	mov si, hello
	call pstr

	call enable_a20

	call rdisk
	mov si, readsk
	call pstr

	mov si, 0x1200
	call pstr

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

enable_a20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

rdisk:
	xor ax, ax
	mov es, ax
	mov bx, 0x1200
	mov ah, 0x02
	mov al, 1
	mov ch, 0
	mov cl, 2
	mov dh, 0
	mov dl, 0x80
	int 0x13
	jc .err
.err:
	mov si, err
	call pstr
	cli
	hlt
	jmp $

hello db "Hello World from TEST OS!", 13, 10, 0
readsk db "Finished reading disk!", 13, 10, 0
err db "Failed to read disk!", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xaa55
