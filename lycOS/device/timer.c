#include "timer.h"
#include "../memory/memory.h"
#include "../int/int.h"
#include "../bootpack.h"

//struct TIMER_ENTRY *entries;
//int size = 0;

// 系统计时器启动经过的时间(毫秒)
unsigned long long current_time;

//struct TIMER_ENTRY {
//    unsigned long long target_time;
//    unsigned int data;
//    struct TIMER_ENTRY* next, prev;
//};
//
void init_timer() {
    current_time = 0;
//    entries = (struct TIMER_ENTRY*) memman_alloc_4k(sys_memman, TIMER_SIZE * sizeof(TIMER_ENTRY));
}
//
//int add_timer(unsigned long long time, unsigned int data) {
//
//}

// 初始化pit (可编程间隔型计时器)
void init_pit() {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    // 此处设置 0x2e9c = 11932, 约等于每秒 100 次中断
}

// 计时器的中断处理
void int_handler20(int *esp) {
    current_time += 10;
    io_out8(PIC0_OCW2, 0x60);  // 中断处理完成
}

