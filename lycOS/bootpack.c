#include "bootpack.h"
#include <stdio.h>
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
#include "multitask/multitask.h"


//unsigned int fifo_data[4096];
struct FIFO32_BUF sys_buf;


struct LAYER* test;
int counter = 0;
void multitask_test() {
    while (1) {
        counter++;
        if (counter % 100000 == 0) {
            char temp[100];
            sprintf(temp, "task1: %d", counter);
            put_ascii_str8_bg(test->content, test->width, 8, ((struct BOOTINFO*) BOOTINFO_ADDR)->scrny - 16 * 10, COLOR8_WHITE, temp, COLOR8_BLACK);
        }
    }
}
int counter2 = 0;
void multitask_test2() {
    while (1) {
        counter2++;
        if (counter2 % 100000 == 0) {
            char temp[100];
            sprintf(temp, "task2: %d", counter2);
            put_ascii_str8_bg(test->content, test->width, 8, ((struct BOOTINFO*) BOOTINFO_ADDR)->scrny - 16 * 9, COLOR8_WHITE, temp, COLOR8_BLACK);
        }
    }
}


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

    // 初始化 gdt, idt, pic
    init_gdtidt();
    init_pic();
    write_serial_str("gdt, idt initialization ok.\r\n");

    // 检查内存并初始化内存管理
    unsigned int memory_total = memtest_sub(0x00400000, 0xbfffffff);  // 最多读取到 3072 MB 内存
    memman_init(sys_memman);
    memman_free(sys_memman, 0x00400000, memory_total - 0x00400000);

    sprintf(temp, "memory manager init ok. total memory: %d MB free: %d MB\r\n",
            memory_total / (1024 * 1024), memman_available(sys_memman) / (1024 * 1024));
    write_serial_str(temp);

    // 初始化计时器
    init_timer();
    init_pit();
    write_serial_str("timer init ok.\r\n");

    // 多任务
    struct TASK *t_main = task_init();
    write_serial_str("multitask init.\r\n");

    // 系统缓冲区分配 (键盘, 鼠标, pit)
    unsigned int *fifo_data = (unsigned int*) memman_alloc(sys_memman, 4096 * sizeof(unsigned int));
    if (fifo_data == NULL) {
        write_serial_str("ERROR: fifo data alloc failed!\r\n");
    }
    fifo32_init(&sys_buf, 4096, fifo_data, t_main);
    write_serial_str("sys buffer initialization ok.\r\n");

    // TODO: 启动时可能卡在这里
    io_sti();  // CPU 接收中断
    io_out8(PIC0_IMR, 0xf8);  // 11111000 接收 PIT, PIC1 和键盘中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 接收鼠标中断
    write_serial_str("ready to process int.\r\n");

    // 初始化键盘鼠标
    init_keyboard();
    enable_mouse(&mouse_dec);
    write_serial_str("keyboard & mouse initialization ok.\r\n");

    // 初始化调色板
    init_palette();
    write_serial_str("palette init ok.\r\n");

    // 初始化图层管理器
    struct LAYERCTL* layerctl = (struct LAYERCTL*) memman_alloc(sys_memman, sizeof(struct LAYERCTL));
    init_layerctl(layerctl, binfo->scrnx, binfo->scrny);

    // 初始化桌面背景层
    struct LAYER* bg_layer = alloc_layer(layerctl, binfo->scrnx, binfo->scrny, 0, 0);

    test = bg_layer;

    // 填充桌面背景色
    box_fill8(bg_layer->content, bg_layer->width, COLOR8_LIGHT_DARK_BLUE, 0, 0, bg_layer->width, bg_layer->height);

    // 显示 GUI HelloWorld!
    put_ascii_str8(bg_layer->content, bg_layer->width, 8, 8, COLOR8_WHITE, "HelloWorld from lycOS!");

    // 鼠标绘制
    struct LAYER* mouse_layer = alloc_layer(layerctl, 8, 16, 0, 0);
    box_fill8(mouse_layer->content, mouse_layer->width, COLOR8_BLACK, 0, 0, 8, 16);
    // TODO: 鼠标图标

    // 图像刷新计时器: 30 fps
    add_timer(33, 1);

    // 多任务测试
    struct TASK *t = new_task();
    t->tss.esp = memman_alloc_4k(sys_memman, 64 * 1024) + 64 * 1024;
    t->tss.eip = (int) &multitask_test;
    t->tss.es = 8;
    t->tss.cs = 8 * 2;
    t->tss.ss = 8;
    t->tss.ds = 8;
    t->tss.fs = 8;
    t->tss.gs = 8;

    run_task(t, 1, 1);

    struct TASK *t2 = new_task();
    t2->tss.esp = memman_alloc_4k(sys_memman, 64 * 1024) + 64 * 1024;
    t2->tss.eip = (int) &multitask_test2;
    t2->tss.es = 8;
    t2->tss.cs = 8 * 2;
    t2->tss.ss = 8;
    t2->tss.ds = 8;
    t2->tss.fs = 8;
    t2->tss.gs = 8;

    run_task(t2, 1, 10);

    write_serial_str("enter mainloop.\r\n");
    while(1){
        io_cli();  // 处理过程中禁止中断

        if (fifo32_data_available(&sys_buf) == 0) {
            io_sti();
            task_sleep(t_main);  // 主动休眠
//            io_stihlt();  // 接收中断并等待
        } else {
            int data = fifo32_get(&sys_buf);

            if (data == 1) {
                add_timer(33, 1);
                layerctl_draw(layerctl, binfo->vram);
            } else if (data >= KEYBOARD_DATA_MIN && data <= KEYBOARD_DATA_MAX) {
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
                }
            } else {
                sprintf(temp, "Data in sys buf: %u\r\n", data);
                write_serial_str(temp);

            }
        }
    }
}


