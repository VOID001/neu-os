/*
 * File to show mm functional and test mm
 *
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <serial_debug.h>

#define invalidate() \
    __asm__ volatile("mov %%eax, %%cr3"::"a" (0))

unsigned long put_page(unsigned long page, unsigned long address);

void testoom() {
    for(int i = 0; i < 20 * 1024 * 1024; i += 4096)
        do_no_page(0, (unsigned long)i);
    // This should not return!
    return ;
}

// This function is currently buggy
// Cannot use
// void test_share_page() {
//     // Copy a page then try to write to it
//     copy_page_tables(0x0, 0x4000000, 0xA0000);
//     mm_print_pageinfo(0x4000000);
//     // write to the page at 0x4000000
//     char *w = 0x4000000;
//     *w = 'p';
//     mm_print_pageinfo(0x4000000);
//     while (1);
// }

void test_put_page() {
    char *b = (char *)0x100000;
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
    if(!(*pde & 1) || (unsigned long)pde > 0x1000) {
        return 0;
    }
    // Now it is page table address :P
    pde = (unsigned long *)(*pde & 0xfffff000);
    // Page table address + page_table index = PTE
    //
    // Remember: x >> 12 & 0x3ff != x >> 10 & 0xffc (后者比前者二进制末尾多了两个0)
    // s_printk("pde = %d\n", (unsigned long)pde + 4);
    // s_printk("pte.offset = %d\n", (unsigned long)pde + (((addr >> 12) & 0x3ff) << 2));
    return (unsigned long *)((unsigned long)(pde) + (((addr >> 12) & 0x3ff) << 2));
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

void mm_read_only(unsigned long addr) {
    unsigned long *pte = linear_to_pte(addr);
    printk("Before: 0x%x\n", *pte);
    *pte = *pte & 0xfffffffd;
    printk("After: 0x%x\n", *pte);
    invalidate();
    return ;
}

void mm_print_pageinfo(unsigned long addr) {
    unsigned long *pte = linear_to_pte(addr);
    s_printk("[DEBUG] Linear addr: 0x%x, PTE addr = 0x%x. Flags[ ", addr, pte);
    if(*pte & 0x1) s_printk("P ");
    if(*pte & 0x2) s_printk("R/W ");
    else s_printk("RO ");
    if(*pte & 0x4) s_printk("U/S ");
    else s_printk("S ");
    s_printk("]\n");
    s_printk("[DEBUG] Phyaddr = %x\n", (*pte & 0xfffff000));
}


int mmtest_main(void) {
    // test_share_page();
    // return 0;
    s_printk("0x%x\n", linear_to_pte(0x1000));
    printk("Running Memory function tests\n");
    printk("1. Make Linear Address 0xdad233 unavailable\n");

    disable_linear(0xdad233);

    // Now don't modify the code above
    // make the linear address AT 0xdad233 accessible
    // and store 0x23333333 at 3MB physical memory
    // (Hint: add a table entry map to physical address 3MB)
    //
    printk("2. Put page(0x300000) at linear address 0xdad233\n");
    put_page(0x300000, 0xdad233);

    unsigned long *x = (unsigned long *)0xdad233;
    *x = 0x23333333;
    printk("X = %x\n", *x);

    // Then make the linear address 0xdad233 Read Only

    printk("3. Make 0xdad233 READ ONLY\n");
    mm_read_only(0xdad233);
    x = (unsigned long *)0xdad233;
    // DO not modify this code
    // Here will disable WP bit (temporarily)
    asm volatile("mov %%cr0, %%eax\n\t"
            "orl $0x00010000, %%eax\n\t"
            "mov %%eax, %%cr0\n\t"
            ::);
    // Please write a GCC Extended assembly here
    // to modify the content at linear address 0xdad233
    // (This will trigger a page write protect fault), uncomment lines below to test
    //asm volatile("mov %%eax, (%%edi)"
    //        ::"D" (x), "a" (0x233111));

    printk("4. Print the page info of 0xdad233 in human readable mode\n");
    mm_print_pageinfo(0xdad233);

    while(1);
    // Please implement
}
