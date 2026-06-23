bits 16
org 0x7c00

KERNEL equ 0x8000

global start
section .text

start:
	jmp _start
	nop

times 33 db 0

_start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x6000

	mov ax, 0x0003
	int 0x10

	mov si, boot
	call print_string
	
	call pause2secs

	mov si, msg
	call print_string
	
	;setting up the a20 gate,
	;now this is so that the bootloader can...
	;...access over 1MiB of RAM
	
	call enable_a20

	mov si, a20
	call print_string

	call pause2secs

	;reading the disk and loading the...
	;...kernel into memory

	call read_dsksec1

	mov al, 'D'
	mov dx, 0x3f8
	out dx, al

	;call pause2secs

	mov si, read_disk_msg
	call print_string

	lgdt [gdt_descriptor]

	call pause2secs

	mov si, gdt
	call print_string

	mov al, 'G'
	mov dx, 0x3f8
	out dx, al

	call pause2secs

	mov si, pm
	call print_string

	call pause2secs

	mov ax, 0x0003
	int 0x10

	cli
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp CODE_SEL:pm_start

pause2secs:
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

read_dsksec1:
	xor ax, ax
	mov es, ax
	mov bx,	KERNEL
	mov ah, 0x02
	mov al, 0x09	;read 2 sectors
	mov ch, 0x00
	mov cl, 0x02
	mov dh, 0x00
	mov dl, 0x80
	int 0x13
	jc .dsk_err
	ret

.dsk_err:
	cli
	hlt
	jmp $-2

print_string:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0e
	int 0x10
	jmp print_string

.done:
	ret

enable_a20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

[bits 32]
pm_start:
	cli
	mov ebp, 0x130000
	mov ax, DATA_SEL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, ebp
	
	mov al, 'P'
	mov dx, 0x3f8
	out dx, al
	
	mov eax, KERNEL ;direct addressing mode
	jmp eax

	cli
	hlt
	jmp $-2

gdt_start:
	gdt_null:
		;null descriptor, must always be zeroed
		dd 0x00000000
		dd 0x00000000
	gdt_code:
		dw 0xffff
		dw 0x0000
		db 0x00
		db 0b10011010
		db 0b11001111
		db 0x00
	gdt_data:
		dw 0xffff
		dw 0x0000
		db 0x00
		db 0b10010010
		db 0b11001111
		db 0x00
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEL equ 0x08
DATA_SEL equ 0x10

msg db "Welcome to BIKT OS!", 13, 10, 0
read_disk_msg db "Finished Reading Disk!", 13, 10, 0
pm db "Entering Protected Mode...Loading Kernel...", 13, 10, 0
dsk_err db "Failed to Read Sector 1 of disk!", 0
boot db "Booting from Hard Disk...", 13, 10, 0
a20 db "Enabled the A20 Gate!", 13, 10, 0
gdt db "Setup the Global Descriptor Table!", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xaa55
