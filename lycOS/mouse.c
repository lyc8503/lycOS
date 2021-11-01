#include "mouse.h"
#include "keyboard.h"
#include "bootpack.h"
#include "int.h"
#include "buffer.h"


struct FIFO8_BUF mouse_buf;

struct MOUSE_DECODE mouse_dec;

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
void enable_mouse(struct MOUSE_DECODE *m_dec) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  // 下一条指令发送给鼠标
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);  // 启动鼠标
    m_dec->phase = 0;
    return;  // 应答信息应该是通过中断发送的 ACK(0xfa)
}

int mouse_decode(struct MOUSE_DECODE *m_dec, unsigned char data) {
    if (m_dec->phase == 0) {
        if (data == 0xfa) {  // 等待初始化完成 ACK 信号
            m_dec->phase = 1;
        }
        return 0;
    }
    if (m_dec->phase == 1) {  // 第一字节
        if ((data & 0xc8) == 0x08) {  // 舍去错误数据
            m_dec->buf[0] = data;
            m_dec->phase = 2;
        }
        return 0;
    }
    if(m_dec->phase == 2) {  // 第二字节
        m_dec->buf[1] = data;
        m_dec->phase = 3;
        return 0;
    }
    if(m_dec->phase == 3) {  // 第三字节
        m_dec->buf[2] = data;
        m_dec->phase = 1;

        m_dec->button = m_dec->buf[0] & 0x07;  // 低三位是按键
        m_dec->x = m_dec->buf[1];
        m_dec->y = m_dec->buf[2];

        if ((m_dec->buf[0] & 0x10) != 0) {  // 依据方向进行符号扩展
            m_dec->x |= 0xffffff00;
        }
        if ((m_dec->buf[0] & 0x20) != 0) {
            m_dec->y |= 0xffffff00;
        }

        m_dec->y = - m_dec->y;  // 鼠标 y 方向与屏幕恰好相反(屏幕 y 轴向下)

        return 1;
    }
    return -1;
}
