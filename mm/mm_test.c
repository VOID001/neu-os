/*
 * File to show mm functional and test mm
 *
 */

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

