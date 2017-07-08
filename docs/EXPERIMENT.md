## 0 环境 SSR

* 搭建环境 ( Virtualbox 跑通 / 自己搭建 )
* 熟悉环境（GCC, GNU Toolchain, Objdump, dd, Makefile, Bochs, QEMU)

## 1 启动
* 启动扇区 (QEMU 运行 Makefile 构建的 bootsector, 并且使用 Objdump 看反汇编)
* [选] 软盘中断 / 硬盘中断
* 硬盘启动 [选]
* 设置 GDT IDT 并查看(通过 Bochs 查看设置是否正确)

## 2 VGA Serial
* 实现 printk （int, int-oct, int-hex, char, string)
* [选] 实现滚屏，清屏，输出彩色字符 ()
* 提供好串口

## 3 Page
* Page Translation  & It's Data Structure PTE PDE GDT
* Bitmap / Buddy 分配 管理
* Page Fault 体验

## 4 Interrupt
* IDT, Trap Gate 等 实现时钟中断
* 实现 die 函数

## 5 Process
* 调度算法 Round Robin, FCFS, 多级队列
* fork 的实现(简单 fork 实现)(不带 COW 的 fork)
* task_struct （进程的组织，链表，数组。）
* execve 的实现 （a.out) [选做]

## 6 System Call & Signal
* 系统调用的处理过程
* 实现一个系统调用
* 信号的机制
* 在 Linux 下自己写一个带信号处理的程序

## 7 Drivers
* 键盘 （译码，读取）
* 终端（队列）（回显，缓冲队列，终端接口）
* 终端的数据结构
* IDE 硬盘 / 软盘驱动 [选]
* 声卡驱动 [选]

## 8 Filesystem (待定)


## 9 Virtual Memory (后续提高)




----
### Draft
1. Roadmap  [DONE]
2. 实验的文档中要有每一个步骤需要的原理/知识的资料提供
3. 实验环境文档要清晰
4. 实验的验证
5. 给小例子，让他完善。（每一个模块都是这样？）
6. 重点
 - [6.1] 用户态内核态切换()
 - [6.2] 进程创建(fork)
 - [6.3] 中断的实现（键盘，时钟）
 - [6.4] 启动过程形象化
 - [6.5] 存储 分页管理，页表
 - [6.6] 系统调用
 - 6.7 中断在系统中的重要性。
 - 6.8 存储 虚拟存储（**暂无**）
 - 6.9 VGA / 键盘
7. 先小例子，让学生实现
8. 提供屏蔽部分底层实现的操作系统资源操作接口（如get_page free_page task_struct)

## 实验设计可以参考的书籍

* 《自己动手做操作系统》 Orange
* Linux 内核设计的艺术
* Linux 0.11 内核完全解读
* 操作系统的设计与实现 陈文智
