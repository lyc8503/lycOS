#ifndef LYCOS_C_MOUSE_H
#define LYCOS_C_MOUSE_H

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define MOUSE_DATA_MIN 512
#define MOUSE_DATA_MAX 767
#define MOUSE_DATA_BIAS 512

extern struct MOUSE_DECODE mouse_dec;

struct MOUSE_DECODE {
    unsigned char buf[3], phase;
    int x, y, button;
};

void int_handler2c(int *esp);
void enable_mouse(struct MOUSE_DECODE *m_dec);
int mouse_decode(struct MOUSE_DECODE *m_dec, unsigned char data);

#endif
