qemu-system-i386 -drive format=raw,file=Bikt_OS.img -boot order=c,menu=on -m 512 -smp 2 -usb -usbdevice tablet -display gtk -serial stdio -no-reboot -d int
