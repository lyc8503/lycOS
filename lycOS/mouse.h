#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

extern struct FIFO8_BUF mouse_buf;

void int_handler2c(int *esp);
void enable_mouse();

