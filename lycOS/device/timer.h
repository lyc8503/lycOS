#ifndef LYCOS_C_TIMER_H
#define LYCOS_C_TIMER_H

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_SIZE 1000

extern unsigned long long current_time;

void init_pit();
void int_handler20(int *esp);

void init_timer();
void add_timer(unsigned long long time, unsigned char data);

#endif
