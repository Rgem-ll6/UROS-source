bits 32

section .text
global start
extern kmain

start:
	cld

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov esp, 0x130000
	xor ebp, ebp

	;mov al, 'K'
	;mov dx, 0x3f8
	;out dx, al

	call kmain

.hang:
	cli
	hlt
	jmp .hang

times 512 - ($ - $$) db 0
