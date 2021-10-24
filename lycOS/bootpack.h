struct BOOTINFO{
    char cyls, leds, vmode, reserve;  // 扇区数, 键盘指示灯状态, 显卡色彩模式
    short scrnx, scrny;  // 分辨率
    char *vram;
};

#define BOOTINFO_ADDR 0x00000ff0

void io_hlt();
void io_cli();
void io_out8(int port, int data);
int io_get_eflags();
void io_set_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);