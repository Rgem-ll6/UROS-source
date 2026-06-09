#include <stdint.h>
#include <stddef.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define _VGA_ADDRESS 0xB8000
#define _VGA_COLS 80
#define _VGA_ROWS 25
#define _VGA_ATTR 0x0E

static volatile u16 *vga_buffer = (volatile u16 *)_VGA_ADDRESS;
static long unsigned int cursor_pos = 0;

void vga_put_c(char c)
{
	if (c == '\n')
	{
		cursor_pos = (cursor_pos / _VGA_COLS + 1) * _VGA_COLS;
	} else {
		vga_buffer[cursor_pos] = (u16)c | ((u16)_VGA_ATTR << 8);
		cursor_pos++;
	}
} 

void vga_puts(const char* str)
{
	while(*str)
	{
		vga_put_c(*str++);
	}
}

void kmain(void)
{
	vga_puts("BIKT-OS Kernel!\n");
	vga_puts("Setting up requirements...\n");
	vga_puts("Setting up Interrupt Descriptor Table...\n");

	while(1){
		__asm__("hlt");
	}
}
