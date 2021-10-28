#define BUFFER_OVERFLOW_FLAG 0x00000001
#define RET_OK 0
#define RET_OVERFLOW -1
#define RET_EMPTY -2

struct FIFO8_BUF {
    unsigned char *buf;
    int next_write, next_read, size, free, flags;
};

void fifo8_init(struct FIFO8_BUF *fifo8, int size, unsigned char* buf);
int fifo8_put(struct FIFO8_BUF *fifo8, unsigned char data);
int fifo8_get(struct FIFO8_BUF *fifo8);
int fifo8_data_available(struct FIFO8_BUF *fifo8);
