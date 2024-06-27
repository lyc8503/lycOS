#include "timer.h"
#include <stdio.h>
#include "../memory/memory.h"
#include "../int/int.h"
#include "../bootpack.h"


struct TIMERCTL *sys_timerctl;

struct TIMER* find_available_timer() {
    int i = 0;
    for (i = 0; i < TIMER_SIZE; i++) {
        if (!sys_timerctl->timers[i].using_flag) {
            sys_timerctl->timers[i].using_flag = 1;
            return &sys_timerctl->timers[i];
        }
    }
    return NULL;
}

void init_timer() {
    // 禁止中断
    int eflags = io_get_eflags();
    io_cli();

    sys_timerctl = (struct TIMERCTL*) memman_alloc_4k(sys_memman, sizeof(struct TIMERCTL));
    sys_timerctl->current_time = 0;

    // 哨兵
    sys_timerctl->timer0 = &sys_timerctl->timers[0];
    sys_timerctl->timer0->using_flag = 1;
    sys_timerctl->timer0->target_time = 0x12345678;
    sys_timerctl->timer0->next_timer = NULL;
    io_set_eflags(eflags);
}

int add_timer(unsigned int time, unsigned int data) {
    // 禁止中断
    int eflags = io_get_eflags();
    io_cli();

    if (time <= 0 || time >= 0xffffffff - sys_timerctl->current_time) {
        io_set_eflags(eflags);
        return TIMER_INVALID_ARG;
    }

    struct TIMER *t = find_available_timer();
    if (t == NULL) {
        io_set_eflags(eflags);
        return TIMER_SIZE_FULL;
    }

    // time 为目标时间
    time += sys_timerctl->current_time;

    // 为第一个
    if (sys_timerctl->timer0->target_time > time) {
        t->next_timer = sys_timerctl->timer0;
        sys_timerctl->timer0 = t;
    } else {
        // 寻找插入位置并插入
        struct TIMER *s = sys_timerctl->timer0;
        while (s->next_timer->target_time < time) {
            s = s->next_timer;
        }
        t->next_timer = s->next_timer;
        s->next_timer = t;
    }

    // 设置相关信息
    t->using_flag = 1;
    t->target_time = time;
    t->data = data;

    // 恢复中断
    io_set_eflags(eflags);
    return 0;
}

// 初始化pit (可编程间隔型计时器)
void init_pit() {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    // 此处设置 0x2e9c = 11932, 约等于每秒 100 次中断
}

// 计时器的中断处理
void int_handler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 中断处理完成

    sys_timerctl->current_time += 10;

    ASSERT(sys_timerctl->current_time < 0xffffff00);

    // 多任务切换
    char task_switch_flag = 0;

    while (sys_timerctl->current_time >= sys_timerctl->timer0->target_time) {

        sys_timerctl->timer0->using_flag = 0;

        if (sys_timerctl->timer0->data != TASK_SWITCH_TIMER_DATA) {
            fifo32_put(&sys_buf, sys_timerctl->timer0->data);
        } else {
            task_switch_flag = 1;
        }

        sys_timerctl->timer0 = sys_timerctl->timer0->next_timer;
    }

    if (task_switch_flag) {  // 要用 flag 的原因是在中断处理过程中调用任务切换可能导致 IF 设为 1, 中断嵌套产生问题
        task_switch();
    }
}

