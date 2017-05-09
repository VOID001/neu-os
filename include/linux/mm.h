#ifndef _MM_H
#define _MM_H

#define PAGE_SIZE 4096

/* extern */ unsigned long get_free_page(void);
/* extern */ unsigned long put_page(unsigned long page, unsigned long address);
/* extern */ void free_page(unsigned long addr);
/* extern */ void calc_mem(void);
void do_no_page(unsigned long error_code, unsigned long address);
int copy_page_tables(unsigned long from, unsigned long to, unsigned long size);
void mm_print_pageinfo(unsigned long addr);

#endif
