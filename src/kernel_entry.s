bits 32
org 0x0

section .text
global start
start:
	cld

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov esp, 0x130000
	
	mov byte [0xB8000], 'K'
	mov byte [0xB8001], 0x0e
	mov byte [0xB8002], 'M'
	mov byte [0xB8003], 0x0e
	mov byte [0xB8004], 'A'
	mov byte [0xB8005], 0x0e
	mov byte [0xB8006], 'I'
	mov byte [0xB8007], 0x0e
	mov byte [0xB8008], 'N'
	mov byte [0xB8009], 0x0e

.hang:
	cli
	hlt
	jmp .hang

times 512 - ($ - $$) db 0
