#include "serial.h"
#include "../bootpack.h"

// 串口, 参考: https://wiki.osdev.org/Serial_Ports
// 暂时没有用输入功能, 传入数据的中断会被忽略

// COM1 初始化
int init_serial() {
    io_out8(SERIAL_PORT + 1, 0x00);    // 禁用所有中断
    io_out8(SERIAL_PORT + 3, 0x80);    // 启用 DLAB (set baud rate divisor)
    io_out8(SERIAL_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    io_out8(SERIAL_PORT + 1, 0x00);    //                  (hi byte)
    io_out8(SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    io_out8(SERIAL_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    io_out8(SERIAL_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    io_out8(SERIAL_PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
    io_out8(SERIAL_PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if(io_in8(SERIAL_PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    io_out8(SERIAL_PORT + 4, 0x0F);
    return 0;
}

int serial_received() {
    return io_in8(SERIAL_PORT + 5) & 0x01;
}

char read_serial() {
    while (serial_received() == 0);
    return io_in8(SERIAL_PORT);
}

int is_transmit_empty() {
    return io_in8(SERIAL_PORT + 5) & 0x20;
}

void write_serial(char c) {
    while (is_transmit_empty() == 0);
    io_out8(SERIAL_PORT, c);
}

void write_serial_str(char* s) {
    while (*s != '\0') {
        write_serial(*s++);
    }
}
