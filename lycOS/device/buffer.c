#include "buffer.h"

void fifo32_init(struct FIFO32_BUF *fifo32, int size, unsigned int* buf) {
    fifo32->buf = buf;
    fifo32->flags = 0;
    fifo32->free = size;
    fifo32->size = size;
    fifo32->next_read = 0;
    fifo32->next_write = 0;
}

// 手写循环链表
int fifo32_put(struct FIFO32_BUF *fifo32, unsigned int data) {

    // 没有空间
    if(fifo32->free == 0) {
        fifo32->flags |= BUFFER_OVERFLOW_FLAG;
        return BUFFER_RET_OVERFLOW;
    }

    // 回到开始写入
    if (fifo32->next_write == fifo32->size) {
        fifo32->next_write -= fifo32->size;
    }

    // 写入操作
    fifo32->buf[fifo32->next_write++] = data;

    // 成功返回
    fifo32->free--;
    return 0;
}

// 循环链表读取
unsigned int fifo32_get(struct FIFO32_BUF *fifo32) {

    if(fifo32->free == fifo32->size) {
        return BUFFER_RET_EMPTY;
    }

    // 下次读取从头开始
    if (fifo32->next_read == fifo32->size) {
        fifo32->next_read -= fifo32->size;
    }

    int data = fifo32->buf[fifo32->next_read++];
    fifo32->free++;

    return data;
}

int fifo32_data_available(struct FIFO32_BUF *fifo32) {
    return fifo32->size - fifo32->free;
}
