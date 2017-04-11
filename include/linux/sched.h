#ifndef _SCHED_H
#define _SCHED_H

// 最多有 64 个进程（任务）同时处于系统中
#define NR_TASKS 64
// 时钟频率 100 hz
#define HZ 100

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#include<linux/mm.h>
#include<linux/head.h>

// 定义任务状态

#define TASK_RUNNING 0
#define TASK_INTERRUPTIBLE 1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_ZOMBIE 3
#define TASK_STOPPED 4

#ifndef NULL
#define NULL ((void *)(0))
#endif

extern int copy_page_tables(unsigned long from, unsigned long to, unsigned long size);
extern int free_page_tables(unsigned long from, unsigned long size);

#endif
