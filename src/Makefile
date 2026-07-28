# Toolchain setup
AS = nasm
CC = gcc
LD = /usr/bin/ld

CFLAGS  = -ffreestanding -fno-pie -fno-stack-protector -nostdlib -mgeneral-regs-only -m32
LDFLAGS = -m elf_i386 -T linker.ld

BOOT_BIN    = brub.bin
ENTRY_OBJ   = entry.o
KERNEL_OBJ  = kernel.o
KERNEL_ELF  = kernel.elf
KERNEL_BIN  = kernel.bin
OS_IMAGE    = Bikt_OS.img

.PHONY: all qemu clean

all: $(OS_IMAGE)

$(BOOT_BIN): boot.s
	$(AS) -f bin $< -o $@

$(ENTRY_OBJ): kernel_entry.s
	$(AS) -f elf32 $< -o $@

$(KERNEL_OBJ): kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(ENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) $(ENTRY_OBJ) $(KERNEL_OBJ) -o $@

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	qemu-img create -f raw $@ 10M
	dd if=$(BOOT_BIN) of=$@ conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc status=none

#if emulating change the dummy.img to your virtual hard disk...
#and the main filesystem has to be formatted to the FAT32 format, sorry for the constraint...
qemu: $(OS_IMAGE)
	qemu-system-i386 \
		-drive format=raw,file=$(OS_IMAGE),index=0,media=disk \
		-drive format=raw,file=dummy.img,index=1,media=disk \
		-boot order=c,menu=on \
		-m 4G -smp 2 -usb -usbdevice tablet \
		-serial stdio -enable-kvm -display gtk

clean:
	rm -f *.o *.bin *.elf *.img
