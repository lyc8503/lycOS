#include <stdio.h>
#include "bootpack.h"
#include "gui/graphic.h"
#include "int/dsctbl.h"
#include "int/int.h"
#include "device/keyboard.h"
#include "device/mouse.h"
#include "device/buffer.h"
#include "memory/memory.h"
#include "gui/layer.h"
#include "device/timer.h"
#include "device/serial.h"


unsigned int fifo_data[4096];
struct FIFO32_BUF sys_buf;

// 系统入口
void MyOSMain() {

    // 初始化串口
    init_serial();
    write_serial_str("lycOS: HelloWorld from serial!\r\n");
    char temp[1000];

    // 读取启动信息
    struct BOOTINFO *binfo = (struct BOOTINFO*) BOOTINFO_ADDR;

    sprintf(temp, "bootinfo addr: %p, cyls: %02X, leds: %02X, vmode: %02X, scrnx: %u, scrny: %u, vram addr: %p\r\n",
            binfo, binfo->cyls, binfo->leds, binfo->vmode, binfo->scrnx, binfo->scrny, binfo->vram);
    write_serial_str(temp);

    if (binfo->scrnx == 320 && binfo->scrny == 200) {
        write_serial_str("warning: VBE features not supported. using 320x200 resolution.\r\n");
    }


    // 初始化 gdt, idt, pic 和 pit
    init_gdtidt();
    init_pic();
    init_pit();

    write_serial_str("gdt, idt, pit initialization ok.\r\n");

    io_sti();  // CPU 接收中断
    io_out8(PIC0_IMR, 0xf8);  // 11111000 接收 PIT, PIC1 和键盘中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 接收鼠标中断

    write_serial_str("ready to process int.\r\n");

    // 初始化调色板
    init_palette();
    int x,y;
    char *vram = 0xe0000000;
    for (x = 0; x < 1280; x++) {
        for (y = 0; y < 1024; y++) {
            vram[y * 1280 + x] = COLOR8_BRIGHT_RED;
        }
    }

    write_serial_str("palette init ok.\r\n");

    // 检查内存并初始化内存管理
    unsigned int memory_total = memtest_sub(0x00400000, 0xbfffffff);  // 最多读取到 3072 MB 内存
    memman_init(sys_memman);
    memman_free(sys_memman, 0x00400000, memory_total - 0x00400000);

    sprintf(temp, "memory manager init ok. total memory: %d MB free: %d MB\r\n",
            memory_total / (1024 * 1024), memman_available(sys_memman) / (1024 * 1024));
    write_serial_str(temp);

    // 初始化计时器
//    init_timer();

    // 初始化图层管理器
    struct LAYERCTL* layerctl = (struct LAYERCTL*) memman_alloc(sys_memman, sizeof(struct LAYERCTL));
    init_layerctl(layerctl, binfo->scrnx, binfo->scrny);

    // 初始化桌面背景层
    struct LAYER* bg_layer = alloc_layer(layerctl, binfo->scrnx, binfo->scrny, 0, 0);

    // 填充桌面背景色
    box_fill8(bg_layer->content, bg_layer->width, COLOR8_LIGHT_DARK_BLUE, 0, 0, bg_layer->width, bg_layer->height);

    // 显示 GUI HelloWorld!
    put_ascii_str8(bg_layer->content, bg_layer->width, 8, 8, COLOR8_WHITE, "HelloWorld from lycOS!");

    // 系统缓冲区分配 (键盘, 鼠标, pit)
    fifo32_init(&sys_buf, 4096, fifo_data);

    init_keyboard();
    enable_mouse(&mouse_dec);

    // 鼠标绘制
    struct LAYER* mouse_layer = alloc_layer(layerctl, 8, 16, 0, 0);
    box_fill8(mouse_layer->content, mouse_layer->width, COLOR8_BLACK, 0, 0, 8, 16);
    // TODO: 鼠标图标

    while(1){
        io_cli();  // 处理过程中禁止中断


        if (fifo32_data_available(&sys_buf) == 0) {
            io_stihlt();  // 接收中断并等待
        } else {
            int data = fifo32_get(&sys_buf);

            if (data >= KEYBOARD_DATA_MIN && data <= KEYBOARD_DATA_MAX) {
                data -= KEYBOARD_DATA_BIAS;

                char output[1024];
                sprintf(output, "KEYBOARD: %02X    ", data);
                put_ascii_str8_bg(bg_layer->content, bg_layer->width, 8, binfo->scrny - 16, COLOR8_WHITE, output, COLOR8_BLACK);
            } else if (data >= MOUSE_DATA_MIN && data <= MOUSE_DATA_MAX) {
                data -= MOUSE_DATA_BIAS;

                if(mouse_decode(&mouse_dec, data) != 0) {
                    char output[1024];
                    sprintf(output, "MOUSE: %d %d %d %d %d    ", mouse_dec.button & 0x01, (mouse_dec.button & 0x04) / 4, (mouse_dec.button & 0x02) / 2, mouse_dec.x, mouse_dec.y);
                    put_ascii_str8_bg(bg_layer->content, bg_layer->width, 8, bg_layer->height - 16, COLOR8_WHITE, output, COLOR8_BLACK);

                    mouse_layer->loc_x += mouse_dec.x;
                    mouse_layer->loc_y += mouse_dec.y;
                    layerctl_draw(layerctl, binfo->vram);
                }
            } else {
                sprintf(temp, "Data in sys buf: %d\r\n", data);
                write_serial_str(temp);
            }
        }
    }
}

