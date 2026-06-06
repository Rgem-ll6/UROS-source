[bits 16]

db "This is the SECTOR OS Kernel!", 13, 10, 0

times 512 - ($ - $$) db 0
