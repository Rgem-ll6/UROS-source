bits 16

db "Hello from Second sector", 13, 10, 0

times 512 - ($ - $$) db 0
