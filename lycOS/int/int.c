#include "int.h"
#include "../bootpack.h"

// 初始化 pic
void init_pic() {

    // 禁止所有中断
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    io_out8(PIC0_ICW1, 0x11);  // 设置边沿触发模式
    io_out8(PIC0_ICW2, 0x20);  // IRQ0-7 由 INT20-27 接收
    io_out8(PIC0_ICW3, 1 << 2);  // PIC1 由 IRQ2 连接
    io_out8(PIC0_ICW4, 0x01);  // 无缓冲区模式

    io_out8(PIC1_ICW1, 0x11);  // 边沿触发模式
    io_out8(PIC1_ICW2, 0x28);  // IRQ8-15 由 INT28-2f 接收
    io_out8(PIC1_ICW3, 2);  // PIC1 由 IRQ2 连接
    io_out8(PIC1_ICW4, 0x01);  // 无缓冲区模式

    io_out8(PIC0_IMR, 0xfb);  // 11111011 禁用除 PIC1 以外中断
    io_out8(PIC1_IMR, 0xff);  // 11111111 禁止全部中断
}

// 忽略 IRQ7
void int_handler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);
}
