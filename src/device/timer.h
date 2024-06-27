#ifndef LYCOS_C_TIMER_H
#define LYCOS_C_TIMER_H

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_SIZE 1000
#define TIMER_SIZE_FULL -1
#define TIMER_INVALID_ARG -2

void init_pit();
void int_handler20(int *esp);

void init_timer();
int add_timer(unsigned int time, unsigned int data);

struct TIMER {
    unsigned int target_time;
    unsigned int data;

    struct TIMER* next_timer;
    int using_flag;
};

struct TIMERCTL {
    unsigned int current_time;
    struct TIMER* timer0;

    struct TIMER timers[TIMER_SIZE];
};

extern struct TIMERCTL* sys_timerctl;

#endif
