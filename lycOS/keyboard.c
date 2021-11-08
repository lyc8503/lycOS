#include "keyboard.h"
#include "bootpack.h"
#include "int.h"
#include "buffer.h"

struct FIFO8_BUF key_buf;

// IRQ1 = 键盘
void int_handler21(int *esp) {
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61);  // 通知 PIC IRO1 已经处理完成

    data = io_in8(PORT_KEYDAT);  // 读取数据
    fifo8_put(&key_buf, data);
}

// 等待键盘控制电路准备完成
void wait_KBC_sendready() {
    while (1) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

// 初始化键盘(及鼠标)控制电路
void init_keyboard() {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}
