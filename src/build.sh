qemu-system-i386 -drive format=raw,file=Bikt_OS.img -boot order=c,menu=on -m 2G,maxmem=4G -smp 4 -usb -usbdevice tablet -display gtk -serial stdio -no-reboot -d int
