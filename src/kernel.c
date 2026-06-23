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
		//if new-line consume all the columns and wrap to next line
	} else {
		vga_buffer[cursor_pos] = (u16)c | ((u16)_VGA_ATTR << 8);
		cursor_pos++;
	}
} 

void vga_puts(const char* str)
{
	//loop till it reaches the null terminator '\0'
	while(*str)
	{
		vga_put_c(*str++);
	}
}

typedef struct {
    u16 offset_low;      // Low 16 bits of handler address
    u16 selector;        // Segment selector(normally 0x08 form our GDT remember?)
    u8 zero;             // Always 0
    u8 type_attr;        // 0x8E = interrupt gate
    u16 offset_high;     // High 16 bits of handler address
} __attribute__((packed)) IDT_Entry;

typedef struct {
    u16 size;            // Size of IDT - 1
    u32 address;         // Address of IDT array
} __attribute__((packed)) IDTR;

#define _IDT_ENTRIES 256
static IDT_Entry idt[_IDT_ENTRIES]; //create the 256 IDT entries

void vga_put_hex(u32 num)
{
    const char *hex = "0123456789ABCDEF";
    
    vga_puts("0x");
    
    // Print each hex digit
    for (int i = 7; i >= 0; i--)
    {
        u8 digit = (num >> (i * 4)) & 0xF;
        vga_put_c(hex[digit]);
    }
}

// Function to set one IDT entry
void idt_set_entry(int index, u32 handler_addr, u16 selector, u8 type_attr)
{
    idt[index].offset_low = handler_addr & 0xFFFF;           // Low 16 bits
    idt[index].offset_high = (handler_addr >> 16) & 0xFFFF;  // High 16 bits
    idt[index].selector = selector;
    idt[index].zero = 0;
    idt[index].type_attr = type_attr;
}

// Load IDT into CPU
void idt_load(void)
{
    IDTR idtr;
    idtr.size = (sizeof(IDT_Entry) * _IDT_ENTRIES) - 1;  // Size - 1
    idtr.address = (u32)&idt;                            // Address of IDT array
    
    // Load IDT using lidt instruction
    __asm__("lidt %0" : : "m"(idtr));
    
    // Enable interrupts globally
    //__asm__("sti"); but already done in the init
}

__attribute__((interrupt)) void divide_by_zero_handler(void *frame)
{
    vga_puts("\n=== INTERRUPT #0: DIVIDE BY ZERO ===\n");
    vga_puts("A program tried to divide by zero!\n");
    vga_puts("System halted.\n");
    
    while(1)
    {
        __asm__("hlt");
    }
}

__attribute__((interrupt)) void debug_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #1: DEBUG ===\n");
	vga_puts("Debug exception triggered (breakpoint/single-step).\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void nmi_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #2: NON-MASKABLE INTERRUPT ===\n");
	vga_puts("Hardware error detected!\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void breakpoint_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #3: BREAKPOINT ===\n");
	vga_puts("Breakpoint hit (int3 instruction).\n");
}

__attribute__((interrupt)) void overflow_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #4: OVERFLOW ===\n");
	vga_puts("Signed integer overflow detected!\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void bound_range_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #5: BOUND RANGE EXCEEDED ===\n");
	vga_puts("Array index out of bounds (BOUND instruction).\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void invalid_opcode_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #6: INVALID OPCODE ===\n");
	vga_puts("CPU executed an unknown instruction!\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void device_not_available_handler(void *frame)
{
	vga_puts("\n=== EXCEPTION #7: DEVICE NOT AVAILABLE ===\n");
	vga_puts("Floating point unit not enabled or missing.\n");
	while(1) __asm__("hlt");
}

// Double Fault has error code - different signature
__attribute__((interrupt)) void double_fault_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #8: DOUBLE FAULT ===\n");
	vga_puts("Processor couldn't handle another exception!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void invalid_tss_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #10: INVALID TSS ===\n");
	vga_puts("Task state segment is invalid!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void segment_not_present_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #11: SEGMENT NOT PRESENT ===\n");
	vga_puts("Segment descriptor not present in memory!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void stack_fault_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #12: STACK-SEGMENT FAULT ===\n");
	vga_puts("Stack segment violation!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1) __asm__("hlt");
}

__attribute__((interrupt)) void general_protection_fault_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #13: GENERAL PROTECTION FAULT ===\n");
	vga_puts("Memory protection violation!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1) __asm__("hlt");
}

// Page Fault has error code
__attribute__((interrupt)) void page_fault_handler(void *frame, u32 error_code)
{
	vga_puts("\n=== EXCEPTION #14: PAGE FAULT ===\n");
	vga_puts("Memory page not present or access violation!\n");
	vga_puts("Error code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	
	// Read faulting address from CR2 register
	u32 faulting_addr;
	__asm__("mov %%cr2, %0" : "=r"(faulting_addr));
	vga_puts("Faulting address: ");
	vga_put_hex(faulting_addr);
	vga_puts("\n");
	
	while(1) __asm__("hlt");
}

void idt_init(void)
{
	__asm__("cli"); //clear interrupts

    vga_puts("Initializing IDT...\n");
    
    // Clear all IDT entries first
    for (int i = 0; i < _IDT_ENTRIES; i++)
    {
        idt_set_entry(i, 0, 0x08, 0x0E);
        //also sets entry type_attr to zero, might send CPU
        //into a triple-fault if not careful
    }
    
    // Set up divide by zero handler
    // Index 0, handler address, selector 0x08 (kernel code), type 0x8E (interrupt gate)
    idt_set_entry(0, (u32)divide_by_zero_handler, 0x08, 0x8E);
   	idt_set_entry(1, (u32)debug_handler, 0x08, 0x8E);
	idt_set_entry(2, (u32)nmi_handler, 0x08, 0x8E);
	idt_set_entry(3, (u32)breakpoint_handler, 0x08, 0x8E);
	idt_set_entry(4, (u32)overflow_handler, 0x08, 0x8E);
	idt_set_entry(5, (u32)bound_range_handler, 0x08, 0x8E);
	idt_set_entry(6, (u32)invalid_opcode_handler, 0x08, 0x8E);
	idt_set_entry(7, (u32)device_not_available_handler, 0x08, 0x8E);
	idt_set_entry(8, (u32)double_fault_handler, 0x08, 0x8E);
	idt_set_entry(10, (u32)invalid_tss_handler, 0x08, 0x8E);
	idt_set_entry(11, (u32)segment_not_present_handler, 0x08, 0x8E);
	idt_set_entry(12, (u32)stack_fault_handler, 0x08, 0x8E);
	idt_set_entry(13, (u32)general_protection_fault_handler, 0x08, 0x8E);
	idt_set_entry(14, (u32)page_fault_handler, 0x08, 0x8E); 

    // Load IDT into CPU
    idt_load();
    
    vga_puts("IDT initialized successfully!\n");
    
    //__asm__("sti"); //start interrupts
}

void kmain(void)
{
	vga_puts("BIKT-OS Kernel!\n");
	vga_puts("Setting up requirements...\n");
	vga_puts("Setting up Interrupt Descriptor Table...\n");
	idt_init();
	vga_puts("Setting up the PIC->Programmable Interrupt Controller\n");

	//vga_puts("Interrupt not kicking?!\n");
	
	while(1){
		__asm__("hlt");
	}
}
