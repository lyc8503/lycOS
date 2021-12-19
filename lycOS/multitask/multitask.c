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
    task->priority = 5;
    taskctl->len_tasks = 1;
    taskctl->current_index = 0;

    taskctl->tasks[0] = task;
    load_tr(task->sel);

    // timer.c 中特别判断了 TASK_SWITCH_TIMER_DATA
    add_timer(5, TASK_SWITCH_TIMER_DATA);

    return task;
}

// 运行一个任务(对已经运行的 task 可修改优先级)
void run_task(struct TASK* task, int priority) {
    if (priority > 0) {
        task->priority = priority;
    }

    if (task->flags != TASK_ACTIVE_FLAG) {
        task->flags = TASK_ACTIVE_FLAG;
        taskctl->tasks[taskctl->len_tasks++] = task;
    }
}

// 切换任务(由 timer.c 调用)
void task_switch() {

    char temp[100];
//    sprintf(temp, "BEFORE: running: %d count: %d fjmp: %d\r\n", taskctl->current_index, taskctl->len_tasks, taskctl->tasks[taskctl->current_index]->sel);
//    write_serial_str(temp);

    taskctl->current_index++;
    if (taskctl->current_index == taskctl->len_tasks) {
        taskctl->current_index = 0;
    }

    sprintf(temp, "running: %d count: %d fjmp: %d priority: %d\r\n",
            taskctl->current_index,
            taskctl->len_tasks,
            taskctl->tasks[taskctl->current_index]->sel,
            taskctl->tasks[taskctl->current_index]->priority);
//    write_serial_str(temp);

    add_timer(taskctl->tasks[taskctl->current_index]->priority * 10, TASK_SWITCH_TIMER_DATA);

    if (taskctl->len_tasks >= 2) {
        farjmp(0, taskctl->tasks[taskctl->current_index]->sel);
    }
}

// 任务休眠
void task_sleep(struct TASK* task) {
    int i = 0, switch_flag = 0;

    if (task->flags != TASK_ACTIVE_FLAG) {
        return;
    }

    // 如果要休眠的是当前任务, 设置 switch_flag = 1
    if (task == taskctl->tasks[taskctl->current_index]) {
        switch_flag = 1;
    }

    // 寻找位置
    for (i = 0; i < taskctl->len_tasks; i++) {
        if (taskctl->tasks[i] == task) {
            break;
        }
    }

    taskctl->len_tasks--;
    // 移动成员
    if (i < taskctl->current_index) {
        taskctl->current_index--;
    }

    for (; i < taskctl->len_tasks; i++) {
        taskctl->tasks[i] = taskctl->tasks[i + 1];
    }

    task->flags = TASK_USING_FLAG;
    if (switch_flag) {
        if (taskctl->current_index >= taskctl->len_tasks) {
            taskctl->current_index = 0;
        }
        farjmp(0, taskctl->tasks[taskctl->current_index]->sel);
    }
}
