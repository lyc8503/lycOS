// 启动时保存的一些信息
struct BOOTINFO{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};

#define BOOTINFO_ADDR 0x0ff0

#define COLOR8_BLACK 0
#define COLOR8_BRIGHT_RED 1
#define COLOR8_BRIGHT_GREEN 2
#define COLOR8_BRIGHT_YELLOW 3
#define COLOR8_BRIGHT_BLUE 4
#define COLOR8_BRIGHT_PURPLE 5
#define COLOR8_LIGHT_BRIGHT_BLUE 6
#define COLOR8_WHITE 7
#define COLOR8_BRIGHT_GREY 8
#define COLOR8_DARK_RED 9
#define COLOR8_DARK_GREEN 10
#define COLOR8_DARK_YELLOW 11
#define COLOR8_DARK_CYAN 12
#define COLOR8_DARK_PURPLE 13
#define COLOR8_LIGHT_DARK_BLUE 14
#define COLOR8_DARK_GREY 15

extern char ascfont[4096];

void io_hlt();
void io_cli();
void io_out8(int port, int data);
int io_get_eflags();
void io_set_eflags(int eflags);


void set_palette(int start, int end, unsigned char *rgb){
    int i, eflags;
    eflags = io_get_eflags(); // 保存当前中断状态
    io_cli(); // 禁止中断

    // https://stackoverflow.com/questions/42814777/asm-what-does-port-3c8h-3c9h-do
    io_out8(0x03c8, start);
    for (i = start; i <= end; i ++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_set_eflags(eflags); // 恢复中断状态
    return;
}

void init_palette(){
    static unsigned char table_rgb[16 * 3] = {
            0x00, 0x00, 0x00,   // 黑色
            0xff, 0x00, 0x00,   // 亮红
            0x00, 0xff, 0x00,   // 亮绿
            0xff, 0xff, 0x00,   // 亮黄
            0x00, 0x00, 0xff,   // 亮蓝
            0xff, 0x00, 0xff,   // 亮紫
            0x00, 0xff, 0xff,   // 浅亮蓝
            0xff, 0xff, 0xff,   // 白
            0xc6, 0xc6, 0xc6,   // 亮灰
            0x84, 0x00, 0x00,   // 暗红
            0x00, 0x84, 0x00,   // 暗绿
            0x84, 0x84, 0x00,   // 暗黄
            0x00, 0x00, 0x84,   // 暗青
            0x84, 0x00, 0x84,   // 暗紫
            0x00, 0x84, 0x84,   // 浅暗蓝
            0x84, 0x84, 0x84    // 暗灰
    };
    
    set_palette(0, 15, table_rgb);
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1){
    int x, y;
    for (y = y0; y <= y1; y ++) {
        for (x = x0; x <= x1; x ++) {
            vram[y * xsize + x] = c;
        }
    }
    return;
}

void put_ascii_font8(unsigned char *vram, int xsize, int x, int y, char c, char *font){
    int i;
    char *p, d;

    // 每次可以写入一字节 = 8 bit = 8 像素
    for (i = 0; i < 16; i ++){
        p = vram + (y + i) * xsize + x;
        d = font[i];
        if ((d & 0x80) != 0) { p[0] = c; }
        if ((d & 0x40) != 0) { p[1] = c; }
        if ((d & 0x20) != 0) { p[2] = c; }
        if ((d & 0x10) != 0) { p[3] = c; }
        if ((d & 0x08) != 0) { p[4] = c; }
        if ((d & 0x04) != 0) { p[5] = c; }
        if ((d & 0x02) != 0) { p[6] = c; }
        if ((d & 0x01) != 0) { p[7] = c; }
    }
    return;
}

void put_ascii_str8(unsigned char *vram, int xsize, int x, int y, char c, unsigned char *str){
    for (; *str != 0x00; str ++) {
        put_ascii_font8(vram, xsize, x, y, c, ascfont + *str * 16);
        x += 8;
    }
}


// 系统入口
void MyOSMain(){

    // 读取启动信息
    struct BOOTINFO *binfo = (struct BOOTINFO *) BOOTINFO_ADDR;


    // 初始化调色板
    init_palette();

    // 填充桌面背景色
    boxfill8(binfo->vram, binfo->scrnx, COLOR8_LIGHT_DARK_BLUE, 0, 0, binfo->scrnx, binfo->scrny);

    // 图片显示 HelloWorld!

    put_ascii_str8(binfo->vram, binfo->scrnx, 8, 8, COLOR8_WHITE, "HelloWorld from lycOS!");

    while(1){
        io_hlt();
    }
}
