#ifndef LYCOS_C_BUFFER_H
#define LYCOS_C_BUFFER_H

#define BUFFER_OVERFLOW_FLAG 0x00000001
#define BUFFER_RET_OVERFLOW -1
#define BUFFER_RET_EMPTY 0xffffffff

struct FIFO32_BUF {
    unsigned int *buf;
    int next_write, next_read, size, free, flags;
};

void fifo32_init(struct FIFO32_BUF *fifo32, int size, unsigned int* buf);
int fifo32_put(struct FIFO32_BUF *fifo32, unsigned int data);
unsigned int fifo32_get(struct FIFO32_BUF *fifo32);
int fifo32_data_available(struct FIFO32_BUF *fifo32);

#endif
