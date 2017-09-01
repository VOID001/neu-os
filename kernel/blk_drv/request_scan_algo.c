/*
 * block driver Request queue implementation
 * using SCAN algorithm
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <linux/fs.h>

#include "blk.h"

struct request req_tbl[NR_REQUEST];
struct task_struct * wait_for_request = NULL;

struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
};

static void add_request(struct blk_dev_struct * dev, struct request * req) {
    struct request * tmp;
    req->next = NULL;
    cli();

    // current disable fs/buffer functional
    if(req->bh) {
        req->bh->b_dirt = 0;
    }

    // If current req == NULL means this is the
    // first request
    if (!(tmp = dev->current_request)) {
        dev->current_request = req;
        sti();
        (dev->request_fn)();
        return ;
    }

    for(; tmp->next != NULL; tmp = tmp->next) {
        if ((IN_ORDER(tmp, req) || 
                    !IN_ORDER(tmp, tmp->next)) 
                && IN_ORDER(req, tmp->next))
            break;
    }
    req->next = tmp->next;
    tmp->next = req;
    sti();
}

static inline void lock_buffer(struct buffer_head *bh) {
    cli();
    while(bh->b_lock) {
        sleep_on(&bh->b_wait);
    }
    bh->b_lock = 1;
    sti();
}

static inline void unlock_buffer(struct buffer_head *bh) {
    if (!bh->b_lock) {
        printk("unlock_buffer: buffer not locked\n"); 
    }
    bh->b_lock = 0;
    wake_up(&bh->b_wait);
}

static void make_request(int major, int rw, struct buffer_head *bh) {
    struct request * req;
    int rw_ahead;
    short available = 0;
    if ((rw_ahead = (rw == READA || rw == WRITEA))) {
        if (bh->b_lock) {
            return ;
        }
        if (rw == READA) {
            rw = READ;
        }
        
        if (rw == WRITEA) {
            rw = WRITE;
        }
    }
    if (!(rw == READ || rw == WRITE)) {
        panic("Bad block dev command, must be R/W/RA/WA");
    }
    lock_buffer(bh);
    if ((rw == WRITE && !bh->b_dirt) || (rw == READ && bh->b_uptodate)) {
        unlock_buffer(bh);
        return ;
    }

    // Do not let WRITE request fill the queue
    while(!available) {
        if (rw == READ) {
            req = req_tbl + NR_REQUEST;
        } else {
            req = req_tbl + ((NR_REQUEST * 2) / 3);
        }

        while(--req >= req_tbl) {
            if (req->dev < 0)   // dev < 0 means a new request
                break;
            available = 1;
        }
        if (available) {
            break;
        }
        // no available found
        if (req < req_tbl) {
            if (rw_ahead) {
                unlock_buffer(bh);
                return;
            }
            sleep_on(&wait_for_request);
        }
    }

    // found an empty request place, insert into
    //
    req->dev = bh->b_dev;
    req->cmd = rw;
    req->errors = 0;
    req->buffer = bh->b_data;
    req->sector = bh->b_blocknr << 1;
    req->nr_sectors = 2;
    req->waiting = NULL;
    req->bh = bh;
    req->next = NULL;
    add_request(major + blk_dev, req);
}

// Low level read/write block
void ll_rw_block(int rw, struct buffer_head *bh) {
    unsigned int major;

    if ((major = MAJOR(bh->b_dev)) >= NR_BLK_DEV
            || !(blk_dev[major].request_fn)) {
        printk("Trying to read nonexistent block_device\n");
        return ;
    }
    make_request(major, rw, bh);
}

void blk_dev_init(void) {
    int i;
    for(i = 0; i < NR_REQUEST; i++) {
        req_tbl[i].dev = -1;
        req_tbl[i].next = NULL;
    }
}

