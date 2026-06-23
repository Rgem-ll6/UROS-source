#include <stdint.h>
#include <stddef.h> //library for uint32_t and the rest...

//abbreviations to the uint data types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define _VGA_ADDRESS 0xB8000 //vga memory address
#define _VGA_COLS 80
#define _VGA_ROWS 25
#define _VGA_ATTR 0x0E //yellow on black

#define _PIC1_COMMAND 0x20
#define _PIC1_DATA    0x21 //master PIC
#define _PIC2_COMMAND 0xA0 
#define _PIC2_DATA    0xA1	//slave PIC

#define _ICW1_INIT    0x11
#define _ICW4_8086    0x01

static volatile u16 *vga_buffer = (volatile u16 *)_VGA_ADDRESS;
static long unsigned int cursor_pos = 0;

void pic_remap(void);
void outb(u16 port, u8 val);

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

//prints text to vga buffer
void vga_puts(const char* str)
{
	while(*str)
	{
		vga_put_c(*str++);
	}
}

//this method remaps the entries since long ago the PIC and Exceptions collide...
//...so this maps the PIC IRQs at different addresses from the CPU Exceptions
void pic_remap(void) {
    // Start initialization sequence
    outb(_PIC1_COMMAND, _ICW1_INIT);
    outb(_PIC2_COMMAND, _ICW1_INIT);

    // Set offsets (Master: 0x20, Slave: 0x28)
    outb(_PIC1_DATA, 0x20);
    outb(_PIC2_DATA, 0x28);

    // Tell Master there is a slave PIC at IRQ2
    outb(_PIC1_DATA, 4);
    // Tell Slave its cascade identity
    outb(_PIC2_DATA, 2);

    // Set mode to 8086/88
    outb(_PIC1_DATA, _ICW4_8086);
    outb(_PIC2_DATA, _ICW4_8086);

    // Mask all interrupts until ready
    outb(_PIC1_DATA, 0xFF);
    outb(_PIC2_DATA, 0xFF);
}

//outb bytes, sends data to ports/hardware(i guess)
void outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

//opposite or outb, reads...
u8 inb(u16 port)
{
	u8 ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

//PIC End Of Interrupt, this tells the PIC when an ISR is over...
//...and it should tell CPU to continuw what is was doing "hlt" hehe
void pic_eoi(int irq)
{
	if (irq >= 8) outb(_PIC2_COMMAND, 0x20);
	outb(_PIC1_COMMAND, 0x20);
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

//various handlers...
//because of __attribute__((interrupt)) we dont have to make an externel ISR stub
//and it auto returns iret not ret so the CPU knows its an interrupt
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

__attribute__((interrupt)) void floating_point_exception_handler(void *frame, u32 error_code)
{
	vga_puts("\n===EXCEPTION #16: FLOATING POINT EXCEPTION ===\n");
	vga_puts("Math Error in the FPU\n");
	vga_puts("Error Code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1)
		__asm__("hlt");
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

__attribute__((interrupt)) void alignment_check_handler(void *frame, u32 error_code)
{
	vga_puts("\n===EXCEPTION #17: ALIGNMENT CHECK ===\n");
	vga_puts("Error Code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1)
		__asm__("hlt");
}

__attribute__((interrupt)) void machine_check_handler(void *frame, u32 error_code)
{
	vga_puts("\n===EXCEPTION #18: MACHINE CHECK ===\n");
	vga_puts("Fatal hardware error!\n");
	vga_puts("Error Code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1)
		__asm__("hlt");
}

__attribute__((interrupt)) void simd_floating_point_exception_handler(void *frame, u32 error_code)
{
	vga_puts("\n===EXCEPTION #19: SIMD FLOATING-POINT EXCEPTION ===\n");
	vga_puts("Math Error in the SSE/AVX\n");
	vga_puts("Error Code: ");
	vga_put_hex(error_code);
	vga_puts("\n");
	while(1)
		__asm__("hlt");
}

static u32 timer_ticks = 0; //timer ticks...

__attribute__((interrupt)) void timer_handler(void *frame)
{
	timer_ticks++;
	pic_eoi(0);
	//we'll make use of this again when making a sleep() command...
}

// The index is the scan code. 0 means "no character".
//current characters the CPU can read from the keyboard
const char kdb_map[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

__attribute__((interrupt)) void keyboard_handler(void *frame)
{
	u8 scancode = inb(0x60);

	if(scancode < 0x80)
	{
		char c = kdb_map[scancode];
		if (c != 0) vga_put_c(c);
	}
	pic_eoi(1);
}

__attribute__((interrupt)) void com1_handler(void *frame)
{
	vga_puts("COM1 Activity!\n");
	pic_eoi(4);
}

__attribute__((interrupt)) void mouse_handler(void *frame)
{
	vga_puts("Mouse move!\n");
	pic_eoi(12);
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
    // Index 0, ISR handler address, code selector 0x08 (kernel code), type 0x8E (interrupt gate)
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
	idt_set_entry(16, (u32)floating_point_exception_handler, 0x08, 0x8E);
	idt_set_entry(17, (u32)alignment_check_handler, 0x08, 0x8E);
	idt_set_entry(18, (u32)machine_check_handler, 0x08, 0x8E);	
	idt_set_entry(19, (u32)simd_floating_point_exception_handler, 0x08, 0x8E);
	idt_set_entry(0x20, (u32)timer_handler, 0x08, 0x8E);
	idt_set_entry(0x21, (u32)keyboard_handler, 0x08, 0x8E);
	idt_set_entry(0x24, (u32)com1_handler, 0x08, 0x8E);
	idt_set_entry(0x2C, (u32)mouse_handler, 0x08, 0x8E);
    // Load IDT into CPU
    idt_load();
    vga_puts("IDT initialized with PIC support!\n");
    
    //__asm__("sti"); //start interrupts
}

#define _COM1 0x3f8

//starts the serial_driver and it listens...
//..like a radio
void serial_init(void)
{
    // 1. Disable all interrupts (we are doing polling for now)
    outb(_COM1 + 1, 0x00);
    // 2. Enable DLAB (set divisor mode)
    outb(_COM1 + 3, 0x80);
    // 3. Set divisor to 3 (38400 baud)
    outb(_COM1 + 0, 0x03);
    outb(_COM1 + 1, 0x00);
    // 4. 8 bits, no parity, one stop bit
    outb(_COM1 + 3, 0x03);
    // 5. Enable FIFO, clear them, with 14-byte threshold
    outb(_COM1 + 2, 0xC7);
    // 6. IRQs enabled, RTS/DSR set
    outb(_COM1 + 4, 0x0B);		
}

//to check if there is 'noise', if there is we can't print
//and we wait for the noise to settle then
int is_transmit_empty(void)
{
	return inb(_COM1 + 5) & 0x20;
}

//prints a character to 0x3f8
void serial_put_c(char c)
{
	while (is_transmit_empty() == 0);
	outb(_COM1, c);
}

//prints text to 0x3f8
void serial_puts(const char* str)
{
	while (*str)
	{
		serial_put_c(*str++);
	}
}

//main kernel function
void kmain(void)
{
	vga_puts("BIKT-OS Kernel!\n");
	vga_puts("Setting up requirements...\n");
	vga_puts("Setting up Interrupt Descriptor Table...\n");
	//set up idt
	idt_init();
	vga_puts("Setting up the PIC->Programmable Interrupt Controller\n");
	//remap all the PIC IRQ entries
	pic_remap();

	//set the Master and Slave bits so the PIC cna currently take...
	//...IRQ 0, 1, 2, 3, 4, 12
	outb(_PIC1_DATA, 0xE8);
	outb(_PIC2_DATA, 0xEF);

	vga_puts("PIC initialized and interrupts enabled\n");
	vga_puts("Setting up Driver Models...\n");
	
	//initialize the serial driver...
	serial_init();
	vga_puts("Serial Driver initialized!\n");
	serial_puts("Serial Driver initialized!\n");

	__asm__("sti"); //start interrupts

	//halt the CPU forever after executing instructions...
	while(1){
		__asm__("hlt");
	}
}
