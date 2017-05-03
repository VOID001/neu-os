/*
 * Code to implement 80386 traps and interrupts
 *
 */

#include <linux/head.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/system.h>

//void page_fault(void);

// mm/page.s
void page_fault(void);

// 在kernel/asm.s中用到的函数原型
void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void coprocessor_error(void);
void irq13(void);
void reserved(void);
void parallel_interrupt(void);

// 用来引发一个异常，因为目前我们没有实现
// 信号机制，且为单进程，所以die的处理就是打印完必要的信息
// 停机进入死循环
static void die(char *str, long esp_ptr, long nr) {
    long *esp = (long *)esp_ptr;
    printk("%s: %x\n", str, nr & 0xffff);
    printk("EIP: 0x%x:0x%x\n EFLAGS: 0x%x\n ESP 0x%x:0x%x\n",
            esp[1], esp[0], esp[2], esp[4], esp[3]);
    // Some Process Related code, now stub
    printk("base 0x%x, limit 0x%x\n", get_base(current->ldt[1]), get_limit(0x17));
    
    printk("No Process now, System HALT!   :(\n");
    for(;;);
    return ;
}

void do_double_fault(long esp, long error_code) {
    die("double fault", esp, error_code);
}

void do_general_protection(long esp, long error_code) {
    die("general protection", esp, error_code);
}

void do_divide_error(long esp, long error_code) {
    die("divide error", esp, error_code);
}

void do_int3(long *esp, long error_code,
        long fs, long es, long ds,
        long ebp, long esi, long edi,
        long edx, long ecx, long ebx, long eax) {
    
    // Now we do not support Task Register
    int tr = 0;
    __asm__ volatile("str %%ax":"=a" (tr):"0"(0));
    
    printk("eax\tebx\tecx\tedx\t\n%x\t%x\t%x\t%x\n",eax, ebx, ecx, edx);
    printk("esi\tedi\tebp\tesp\t\n%x\t%x\t%x\t%x\n",esi, edi, ebp, (long)esp);
    printk("ds\tes\tfs\ttr\n%x\t%x\t%x\t%x\n",ds, es, fs, tr);
    printk("EIP: %x    CS:%x     EFLAGS: %x", esp[0], esp[1], esp[2]);
    printk("errno = %d", error_code);
    return ;
}

void do_nmi(long esp, long error_code) {
    die("nmi", esp, error_code);
}

void do_debug(long esp, long error_code) {
    die("debug", esp, error_code);
}

void do_overflow(long esp, long error_code) {
    die("overflow", esp, error_code);
}

void do_bounds(long esp, long error_code) {
    die("bounds", esp, error_code);
}

void do_invalid_op(long esp, long error_code) {
    die("invalid operand", esp, error_code);
}

void do_device_not_available(long esp, long error_code) {
    die("device not available", esp, error_code);
}

void do_coprocessor_segment_overrun(long esp, long error_code) {
    die("coprocessor segment overrun", esp, error_code);
}

void do_invalid_TSS(long esp, long error_code) {
    die("invalid TSS", esp, error_code);
}

void do_segment_not_present(long esp, long error_code) {
    die("segment not present", esp, error_code);
}

void do_stack_segment(long esp, long error_code) {
    die("stack segment", esp, error_code);
}

void do_coprocessor_error(long esp, long error_code) {
    die("coprocessor error", esp, error_code);
}

void do_reserved(long esp, long error_code) {
    die("reserved(15, 17-47)error", esp, error_code);
}

void do_stub(long esp, long error_code) {
    printk("stub interrupt! %x, %x\n", esp, error_code);
}

void trap_init(void) {
    int i;

    set_trap_gate(0, &divide_error);
    set_trap_gate(1, &debug);
    set_trap_gate(2, &nmi);
    set_system_gate(3, &int3);
    set_system_gate(4, &overflow);
    set_system_gate(5, &bounds);
    set_trap_gate(6, &double_fault);
    set_trap_gate(7, &device_not_available);
    set_trap_gate(8, &double_fault);
    set_trap_gate(9, &coprocessor_segment_overrun);
    set_trap_gate(10, &invalid_TSS);
    set_trap_gate(11, &segment_not_present);
    set_trap_gate(12, &stack_segment);
    set_trap_gate(13, &general_protection);
    set_trap_gate(14, &page_fault);
    set_trap_gate(15, &reserved);
    set_trap_gate(16, &coprocessor_error);
    for(i = 17; i < 48; i++) {
        set_trap_gate(i, &reserved);
    }
    set_trap_gate(45, &irq13);
    set_trap_gate(39, &parallel_interrupt);
}
