#include <stddef.h>
#include <stdint.h>

static void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}
static inline long hyro_hypercall_stop_execution(void) {
    register long rax asm("rax") = 100;  // Assegna 98 a RAX
    asm volatile (
        "vmcall"  // Esegue l'hypercall (Intel VMX)
        : "+r" (rax) // Output (rax può essere modificato)
        :           // Nessun input oltre RAX
        : "rbx", "rcx", "rdx", "memory" // Clobbered registers
    );
    return rax;  // Restituisce il valore di RAX dopo la chiamata
}
void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
	const char *p;

	for (p = "Hello, world!\n"; *p; ++p)
		outb(0xE9, *p);
	//hyro_hypercall_stop_execution();

	*(long *) 0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}
