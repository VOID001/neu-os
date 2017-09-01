/*
 * blk.h block driver commmon data structure and 
 * macros, function prototypes
 */
#ifndef _BLK_H
#define _BLK_H

#define NR_BLK_DEV 7
#define NR_REQUEST 32

// This is the request queue structure
struct request {
    int dev;
    int cmd;
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char * buffer;
    struct task_struct * waiting;
    struct buffer_head * bh;
    struct request * next;
};

struct blk_dev_struct {
    void (*request_fn)(void);
    struct request * current_request;
};

#define IN_ORDER(s1, s2) \
    ((s1)->cmd < (s2)->cmd || (((s1)->cmd == (s2)->cmd) && \
    ((s1)->dev < (s2)->dev) || ((s1)->dev == (s2)->dev && \
     (s1)->sector < (s2)->sector)))


#endif
