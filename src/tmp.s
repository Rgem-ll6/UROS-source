[bits 16]

db "PLease work", 13, 10, 0

times 512 - ($ - $$) db 0
