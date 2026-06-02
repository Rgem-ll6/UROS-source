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
	mov sp, 0x7c00
	
	mov ax, 0x0003
	int 0x10

	mov si, msg
	call print_string

	mov si, stpat
	call print_string
	call enable_a20

	mov si, a20_msg
	call print_string

	mov si, read_disk_msg
	call print_string
	call read_dsksec1

	mov si, rehd
	call print_string

	mov si, val
	call print_string

	mov si, 0x1200
	call print_string

	cli
	hlt
	jmp $

;print string label
print_string:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0e
	mov bh, 0x0e
	int 0x10
	jmp print_string

	;this whole label loops till al = 0
	;ie. till it reaches the null pointer
	;eventually printing all characters in the string
	;at the .data section

.done:
	ret

enable_a20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

read_dsksec1:
	mov ah, 0x02
	mov al, 0x01
	mov ch, 0x00
	mov cl, 0x02
	mov dh, 0x00
	mov dl, 0x80
	mov bx, 0x1200
	int 0x13
	jc .dsk_err
	ret

.dsk_err:
	mov si, dsk_err
	call print_string
	jmp $

msg db "Welcome to BIKT OS...", 13, 10, 0
stpat db "Setting up A20...", 13, 10, 0
a20_msg db "A20 Enabled...", 13, 10, 0
read_disk_msg db "Reading Sector 1/1st Sector of Hard Disk...", 13, 10, 0
rehd db "Successfully Read Sector 1;", 13, 10, 0
val db "Sector 1: ", 0
dsk_err db "Failed to Read Sector 1 of disk", 0

times 510 - ($ - $$) db 0
dw 0xaa55
