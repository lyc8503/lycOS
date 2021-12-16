#include "multitask.h"
#include "../memory/memory.h"
#include "../int/dsctbl.h"
#include "../bootpack.h"
#include "../device/timer.h"
#include <stdio.h>
#include "../device/serial.h"

struct TASKCTL* taskctl;

// 新建任务
struct TASK* new_task() {
    int i;
    struct TASK* t;

    for (i = 0; i < TASK_SIZE; i++) {
        t = &taskctl->task0[i];

        if (t->flags == TASK_FREE_FLAG) {
            t->flags = TASK_USING_FLAG;  // 使用中
            t->tss.eflags = 0x00000202;  // IF = 1
            t->tss.eax = 0;  // 先设置为 0
            t->tss.ecx = 0;
            t->tss.edx = 0;
            t->tss.ebx = 0;
            t->tss.ebp = 0;
            t->tss.esi = 0;
            t->tss.edi = 0;
            t->tss.es = 0;
            t->tss.ds = 0;
            t->tss.fs = 0;
            t->tss.gs = 0;
            t->tss.ldtr = 0;
            t->tss.iomap = 0x40000000;
            return t;
        }
    }

    return NULL;
}

// 多任务初始化, 初始化后调用者本身也成为一个任务(返回值).
struct TASK* task_init() {
    // gdt 设置
    struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*) GDT_ADDR;
    taskctl = (struct TASKCTL*) memman_alloc_4k(sys_memman, sizeof(struct TASKCTL));

    int i;
    for (i = 0; i < TASK_SIZE; i++) {
        taskctl->task0[i].flags = TASK_FREE_FLAG;
        taskctl->task0[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->task0[i].tss, AR_TSS32);
    }

    struct TASK* task = new_task();
    task->flags = TASK_ACTIVE_FLAG;  // 活动中
    taskctl->running_counter = 1;
    taskctl->current_running = 0;

    taskctl->tasks[0] = task;
    load_tr(task->sel);

    // timer.c 中特别判断了 TASK_SWITCH_TIMER_DATA
    add_timer(10, TASK_SWITCH_TIMER_DATA);

    return task;
}

// 运行一个任务
void run_task(struct TASK* task) {
    task->flags = TASK_ACTIVE_FLAG;
    taskctl->tasks[taskctl->running_counter++] = task;
}

// 切换任务(由 timer.c 调用)
void task_switch() {
    add_timer(10, TASK_SWITCH_TIMER_DATA);

    char temp[40];
    sprintf(temp, "BEFORE: running: %d count: %d fjmp: %d\r\n", taskctl->current_running, taskctl->running_counter, taskctl->tasks[taskctl->current_running]->sel);
    write_serial_str(temp);

    if (taskctl->running_counter >= 2) {
        taskctl->current_running++;
        if (taskctl->current_running == taskctl->running_counter) {
            taskctl->current_running = 0;
        }

        sprintf(temp, "running: %d count: %d fjmp: %d\r\n", taskctl->current_running, taskctl->running_counter, taskctl->tasks[taskctl->current_running]->sel);
        write_serial_str(temp);

        farjmp(0, taskctl->tasks[taskctl->current_running]->sel);
    }
}
