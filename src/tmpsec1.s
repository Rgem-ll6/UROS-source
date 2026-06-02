[bits 16]
 
db "TODO: Kernel will be loaded here soon...", 13, 10, 0

times 512 - ($ - $$) db 0
