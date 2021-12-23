#ifndef LYCOS_C_BOOTPACK_H
#define LYCOS_C_BOOTPACK_H

struct BOOTINFO {
    char cyls, leds, vmode, reserve;  // 扇区数, 键盘指示灯状态, 显卡色彩模式
    short scrnx, scrny;  // 分辨率
    unsigned char *vram;
};

#define BOOTINFO_ADDR 0x00000ff0
#include <stdio.h>
#include "device/buffer.h"
#include "device/serial.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

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

void load_tr(int tr);

void farjmp(int eip, int cs);
extern struct FIFO32_BUF sys_buf;

#define printk(format, args...)         \
do {                                    \
    char temp_k[512];                   \
    sprintf(temp_k, format, ##args);    \
    write_serial_str(temp_k);           \
} while(0)

#define ASSERT(expr)                                                                                    \
do {                                                                                                    \
    if (!(expr)) {                                                                                      \
        io_cli();                                                                                       \
        printk("[ASSERT FAILED] file=%s, function=%s, line=%d, caller=%p\r\n",                          \
        __FILE__, __FUNCTION__, __LINE__, __builtin_return_address(0));                                 \
        while (1) io_hlt();                                                                             \
    }                                                                                                   \
} while(0)

#endif
