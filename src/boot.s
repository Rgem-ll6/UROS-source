bits 16
org 0x7c00

global _start
section .text
_start:
	cli
	xor ax, ax	;zero ax
	mov ds, ax	;data segment
	mov es, ax	;extra segment
	mov ss, ax	;stack segment
	mov sp, 0x6000	;stack pointer

	;clear screen
	;mov ax, 0x0003
	;int 0x10
	
	;printing text
	mov si, msg
	call print_string
	
	;setting up the a20 gate,
	;now this is so that the bootloader can...
	;...access over 1MiB of RAM
	
	call enable_a20

	;reading the disk and loading the...
	;...kernel into memory

	call read_dsksec1

	mov si, read_disk_msg
	call print_string
	
	;load gdt into the GDT Register

	lgdt [gdt_descriptor]

	mov si, pm
	call print_string

	mov ax, 0x0003
	int 0x10

	cli
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp CODE_SEL:pm_start

;disk read label
read_dsksec1:
	mov ax, 0x0000
	mov es, ax
	mov bx, 0x8000
	mov ah, 0x02
	mov al, 0x01	;only read 1 sector
	mov ch, 0x00
	mov cl, 0x02
	mov dh, 0x00
	mov dl, 0x80	;prolly 0x80
	;mov bx, 0x8000
	int 0x13
	jc .dsk_err	;jump if carry flag set
	ret

.dsk_err:
	cli
	hlt
	jmp $

[bits 32]
pm_start:
	mov ax, DATA_SEL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x130000
	
	mov al, 'J'
	mov dx, 0x3f8
	out dx, al

	mov al, 'X'
	mov dx, 0x3f8
	out dx, al

	mov eax, 0x8000
	jmp eax

	;infinte loop
	cli
	hlt
	jmp $

;setting up the GDT(Global Descriptor Table)
;so we can jump into PM(Protected Mode/ 32 bits) safely
gdt_start:
	gdt_null:
		;null descriptor, must always be zeroed
		dd 0x00000000
		dd 0x00000000
	gdt_code:
		;code descriptor
		dw 0xffff
		dw 0x0000
		db 0x00
		db 0b10011010
		db 0b11001111
		db 0x00
	gdt_data:
		;data descriptor, almost identical to the
		;code descriptor
		dw 0xffff
		dw 0x0000
		db 0x00
		db 0b10010010
		db 0b11001111
		db 0x00 
gdt_end:

;for the lgdt operation
gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

;constants that we'll use in PM when...
;...resetting the segment registers
CODE_SEL equ 0x08
DATA_SEL equ 0x10

;print string label
print_string:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0e
	int 0x10
	jmp print_string

	;this whole label loops till al = 0
	;ie. till it reaches the null pointer
	;eventually printing all characters in the string
	;at the .data section

.done:
	ret

;a20 label
enable_a20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

;data section
msg db "Welcome to BIKT OS!", 13, 10, 0
read_disk_msg db "Finished Reading Disk!", 13, 10, 0
pm db "Entering Protected Mode...Loading Kernel...", 13, 10, 0
dsk_err db "Failed to Read Sector 1 of disk!", 0

;force padding and boot signature
times 510 - ($ - $$) db 0
dw 0xaa55
