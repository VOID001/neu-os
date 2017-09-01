#ifndef _FS_H
#define _FS_H

#include <linux/sched.h>

#define READ 0
#define WRITE 1
#define READA 2
#define WRITEA 3

struct buffer_head {
    char * b_data;  /* pointer to data block 1024Byte */
    unsigned long b_blocknr; /* block number */
    unsigned short b_dev; /* device no */
    unsigned char b_uptodate;
    unsigned char b_dirt;   /* 0-clean 1-dirty */
    unsigned char b_count;  /* users using this block */
    unsigned char b_lock;   /* 0-unlock 1-lock */
    struct task_struct * b_wait;
    struct buffer_head * b_prev;
    struct buffer_head * b_next;
    struct buffer_head * b_prev_free;
    struct buffer_head * b_next_free;
};

#define MAJOR(a) (((unsigned)(a))>>8)

#endif
