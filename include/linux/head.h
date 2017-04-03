#ifndef _HEAD_H
#define _HEAD_H

typedef struct desc_struct {
    unsigned long a, b;
} desc_table[256];

extern desc_table idt, gdt;

#endif
