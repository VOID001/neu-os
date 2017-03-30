/*
 * File to show mm functional and test mm
 *
 */

#include <linux/kernel.h>

#define invalidate() \
    __asm__ volatile("mov %%eax, %%cr3"::"a" (0))

unsigned long put_page(unsigned long page, unsigned long address);

void testoom() {
    for(int i = 0; i < 20 * 1024 * 1024; i += 4096)
        do_no_page(0, i);
    // This should not return!
    return ;
}

void test_put_page() {
    char *b = 0x100000;
    calc_mem();
    //put_page(0x200000, 0x100000);
    calc_mem();
    *b = 'k';
    while(1);
    return ;
}

// Helper function to convert linear address to PTE
// return physical address on success
// return NULL(0) on failed
unsigned long *linear_to_pte(unsigned long addr) {
    // get the Page Directory Entry first
    // This variable will be used to refer to pte later
    // (for saving memory XD)
    unsigned long *pde = (unsigned long *)((addr >> 20) & 0xffc);
    // Page dir not exist
    // Or the address is not inside the page table address range(<=4KB)
    if(!(*pde & 1) || pde > 0x1000) {
        return 0;
    }
    // Now it is page table address :P
    pde = (unsigned long *)(*pde & 0xfffff000);
    // Page table address + page_table index = PTE
    //
    // Remember: x >> 12 & 0x3ff != x >> 10 & 0xffc (后者比前者二进制末尾多了两个0)
    return pde + ((addr >> 12) & 0x3ff);
}

void disable_linear(unsigned long addr) {
    // Your code here
    // Hint: Modify the correct page table entry
    // then you can make the page Not Present
    
    // Maybe you need a helper function for get
    // the page table entry for a specific address

    unsigned long *pte = linear_to_pte(addr);
    // Disable it
    *pte = 0;
    // Then RELOAD the TLB
    invalidate();
    return ;
}


int mmtest_main(void) {
    int i = 0;
    printk("Running Memory function tests\n");
    printk("1. Make Linear Address 0x200000 unavailable\n");
    disable_linear(0x200000);
    char *x = 0x200000;
    *x = 1;
    // This will cause page fault =w=
    // Computer will reboot again and again
    i = 0;

    while(1);
}
