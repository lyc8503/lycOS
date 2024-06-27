#ifndef LYCOS_C_MULTITASK_H
#define LYCOS_C_MULTITASK_H

#define TASK_SIZE 100
#define LEVEL_SIZE 10
#define TASK_GDT0 3
#define TASK_FULL 1

#define TASK_SWITCH_TIMER_DATA 0xffffffff

#define AR_TSS32 0x0089

#define TASK_FREE_FLAG 0
#define TASK_USING_FLAG 1
#define TASK_ACTIVE_FLAG 2

struct TSS32 {
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};

struct TASK {
    int sel, flags, priority, level;  // sel(selector) 为 GDT 编号
    struct TSS32 tss;
};

struct TASKLEVEL {
    int len_tasks;
    int current_index;
    struct TASK *tasks[TASK_SIZE];
};

struct TASKCTL {
    int current_level, level_change_flag;

    struct TASKLEVEL task_levels[LEVEL_SIZE];
    struct TASK task0[TASK_SIZE * LEVEL_SIZE];
};

struct TASK* new_task();
struct TASK* task_init();
void run_task(struct TASK* task, int level, int priority);
void task_switch();
void task_sleep(struct TASK* task);

#endif
