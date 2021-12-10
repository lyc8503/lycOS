#ifndef LYCOS_C_BOOTPACK_H
#define LYCOS_C_BOOTPACK_H

struct BOOTINFO {
    char cyls, leds, vmode, reserve;  // 扇区数, 键盘指示灯状态, 显卡色彩模式
    short scrnx, scrny;  // 分辨率
    unsigned char *vram;
};

#define BOOTINFO_ADDR 0x00000ff0
#include "device/buffer.h"

// 以下是 naskfunc 中的定义
void io_hlt();
void io_cli();
void io_sti();
void io_stihlt();

void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);

int io_get_eflags();
void io_set_eflags(int eflags);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void asm_inthandler21();
void asm_inthandler27();
void asm_inthandler2c();
void asm_inthandler20();

int get_cr0(void);
void set_cr0(int cr0);

unsigned int memtest_sub(unsigned int start, unsigned int end);

extern struct FIFO32_BUF sys_buf;

#endif
