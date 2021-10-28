#include "mouse.h"
#include "keyboard.h"
#include "bootpack.h"
#include "int.h"
#include "buffer.h"


struct FIFO8_BUF mouse_buf;

// IRQ12 = 鼠标
void int_handler2c(int *esp) {
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64);  // 通知 PIC1 IRO12 处理完成
    io_out8(PIC0_OCW2, 0x62);  // 通知 PIC0 IRQ02 处理完成

    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mouse_buf, data);
    return;
}

// 启用鼠标  鼠标控制电路包含在键盘控制电路中
void enable_mouse() {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  // 下一条指令发送给鼠标
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);  // 启动鼠标
    return;  // 应答信息应该是通过中断发送的 ACK(0xfa)
}
