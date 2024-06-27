#include "multitask.h"
#include <stdio.h>
#include "../memory/memory.h"
#include "../int/dsctbl.h"
#include "../bootpack.h"
#include "../device/timer.h"

struct TASKCTL* taskctl;

void idle_run() {
    while (1) {
        io_hlt();
    }
}

// 当前运行的任务
struct TASK* current_task() {
    struct TASKLEVEL *tl = &taskctl->task_levels[taskctl->current_level];
    return tl->tasks[tl->current_index];
}

// 添加一个任务
void add_task(struct TASK* task) {
    struct TASKLEVEL *tl = &taskctl->task_levels[task->level];

    ASSERT(tl->len_tasks < TASK_SIZE);

    tl->tasks[tl->len_tasks++] = task;
    task->flags = TASK_ACTIVE_FLAG;
}

// 删除一个任务
void remove_task(struct TASK* task) {
    struct TASKLEVEL *tl = &taskctl->task_levels[task->level];
    int i;

    for (i = 0; i < tl->len_tasks; i++) {
        if (tl->tasks[i] == task) {
            break;
        }
    }

    ASSERT(i < tl->len_tasks);

    tl->len_tasks--;
    if (i < tl->current_index) {
        tl->current_index--;
    }

    if (tl->current_index >= tl->len_tasks) {
        tl->current_index = 0;
    }

    task->flags = TASK_USING_FLAG;

    for (; i < tl->len_tasks; i++) {
        tl->tasks[i] = tl->tasks[i + 1];
    }
}

// 切换任务等级
void switch_task_level() {
    int i;
    for (i = 0; i < LEVEL_SIZE; i++) {
        if (taskctl->task_levels[i].len_tasks > 0) {
            break;
        }
    }
    ASSERT(i < LEVEL_SIZE);

    taskctl->current_level = i;
    taskctl->level_change_flag = 0;
}

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
    for (i = 0; i < TASK_SIZE * LEVEL_SIZE; i++) {
        taskctl->task0[i].flags = TASK_FREE_FLAG;
        taskctl->task0[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->task0[i].tss, AR_TSS32);
    }

    for (i = 0; i < LEVEL_SIZE; i++) {
        taskctl->task_levels[i].current_index = 0;
        taskctl->task_levels[i].len_tasks = 0;
    }

    struct TASK* task = new_task();
    task->flags = TASK_ACTIVE_FLAG;  // 活动中
    task->priority = 5;
    task->level = 0;

    add_task(task);
    switch_task_level();  // 设置 current_level
    load_tr(task->sel);

    struct TASK *idle = new_task();
    idle->tss.esp = memman_alloc_4k(sys_memman, 64 * 1024) + 64 * 1024;
    idle->tss.eip = (int) &idle_run;
    idle->tss.es = 1 * 8;
    idle->tss.cs = 2 * 8;
    idle->tss.ss = 1 * 8;
    idle->tss.ds = 1 * 8;
    idle->tss.fs = 1 * 8;
    idle->tss.gs = 1 * 8;

    run_task(idle, LEVEL_SIZE - 1, 1);

    // timer.c 中特别判断了 TASK_SWITCH_TIMER_DATA
    add_timer(5, TASK_SWITCH_TIMER_DATA);

    return task;
}

// 运行一个任务(对已经运行的 task 可修改优先级, level 同理)
void run_task(struct TASK* task, int level, int priority) {

    // 不改变 level
    if (level < 0) {
        level = task->level;
    }

    // 修改优先级
    if (priority > 0) {
        task->priority = priority;
    }

    if (task->flags == TASK_ACTIVE_FLAG && task->level != level) {
        remove_task(task);  // 移出 task, 准备放入新的 level
    }

    // 唤醒或 level 修改
    if (task->flags != TASK_ACTIVE_FLAG) {
        task->level = level;
        add_task(task);
    }

    // 需要检查 level 是否改变
    taskctl->level_change_flag = 1;
}

// 切换任务(由 timer.c 调用)
void task_switch() {

    struct TASKLEVEL *tl = &taskctl->task_levels[taskctl->current_level];
    struct TASK *new_task, *current_task = tl->tasks[tl->current_index];

    tl->current_index++;
    if (tl->current_index == tl->len_tasks) {
        tl->current_index = 0;
    }

    if (taskctl->level_change_flag != 0) {
        switch_task_level();
        tl = &taskctl->task_levels[taskctl->current_level];
    }

//    printk("lv: %d running: %d count: %d fjmp: %d priority: %d\r\n",
//            taskctl->current_level,
//            tl->current_index,
//            tl->len_tasks,
//            tl->tasks[tl->current_index]->sel,
//            tl->tasks[tl->current_index]->priority);

    new_task = tl->tasks[tl->current_index];
    add_timer(new_task->priority * 10, TASK_SWITCH_TIMER_DATA);

    // 原来的任务与新任务是否相同
    if (new_task != current_task) {
        farjmp(0, new_task->sel);
    }
}

// 任务休眠
void task_sleep(struct TASK* task) {
    if (task->flags != TASK_ACTIVE_FLAG) {
        return;
    }

    struct TASK *now_task = current_task();
    remove_task(task);

    // 如果是当前任务需要任务切换
    if (task == now_task) {
        switch_task_level();
        now_task = current_task();
        farjmp(0, now_task->sel);
    }
}
