bits 32

section .text
global start

extern __bss_start, __bss_end
extern kmain

start:
	cld

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;zero out BSS(Uninitialized data) section
	mov edi, __bss_start
	xor eax, eax
	mov ecx, __bss_end
	sub ecx, __bss_start
	shr ecx, 2
	cld
	rep stosd

	call kmain

.hang:
	cli
	hlt
	jmp .hang

times 512 - ($ - $$) db 0
