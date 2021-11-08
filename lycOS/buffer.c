#include "buffer.h"

void fifo8_init(struct FIFO8_BUF *fifo8, int size, unsigned char* buf) {
    fifo8->buf = buf;
    fifo8->flags = 0;
    fifo8->free = size;
    fifo8->size = size;
    fifo8->next_read = 0;
    fifo8->next_write = 0;
}

// 手写循环链表
int fifo8_put(struct FIFO8_BUF *fifo8, unsigned char data) {

    // 没有空间
    if(fifo8->free == 0) {
        fifo8->flags |= BUFFER_OVERFLOW_FLAG;
        return BUFFER_RET_OVERFLOW;
    }

    // 回到开始写入
    if (fifo8->next_write + 1 > fifo8->size - 1) {
        fifo8->next_write -= fifo8->size;
    }

    // 写入操作
    fifo8->buf[fifo8->next_write++] = data;

    // 成功返回
    fifo8->free--;
    return BUFFER_RET_OK;
}

// 循环链表读取
int fifo8_get(struct FIFO8_BUF *fifo8) {

    if(fifo8->free == fifo8->size) {
        return BUFFER_RET_EMPTY;
    }

    // 下次读取从头开始
    if (fifo8->next_read + 1 == fifo8->size - 1) {
        fifo8->next_read -= fifo8->size;
    }

    int data = fifo8->buf[fifo8->next_read++];
    fifo8->free++;

    return data;
}

int fifo8_data_available(struct FIFO8_BUF *fifo8) {
    return fifo8->size - fifo8->free;
}
