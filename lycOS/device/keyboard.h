#ifndef LYCOS_C_KEYBOARD_H
#define LYCOS_C_KEYBOARD_H

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

extern struct FIFO8_BUF key_buf;

void int_handler21(int *esp);
void init_keyboard();
void wait_KBC_sendready();

#endif
