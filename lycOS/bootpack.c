#include <stdio.h>
#include "bootpack.h"
#include "gui/graphic.h"
#include "int/dsctbl.h"
#include "int/int.h"
#include "device/keyboard.h"
#include "device/mouse.h"
#include "device/buffer.h"
#include "memory/memory.h"


unsigned char key_data[128];
unsigned char mouse_data[512];

// 系统入口
void MyOSMain() {
    // 读取启动信息
    struct BOOTINFO *binfo = (struct BOOTINFO *) BOOTINFO_ADDR;

    // 初始化 gdt 和 idt
    init_gdtidt();
    // 初始化 pic
    init_pic();

    io_sti();  // CPU 接收中断
    io_out8(PIC0_IMR, 0xf9);  // 11111001 接收 PIC1 和键盘中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 接收鼠标中断


    // 初始化调色板
    init_palette();

    // 填充桌面背景色
    boxfill8(binfo->vram, binfo->scrnx, COLOR8_LIGHT_DARK_BLUE, 0, 0, binfo->scrnx, binfo->scrny);

    // 显示 HelloWorld!
    put_ascii_str8(binfo->vram, binfo->scrnx, 8, 8, COLOR8_WHITE, "HelloWorld from lycOS!");

    // 检查内存并初始化内存管理
    unsigned int memory_total = memtest_sub(0x00400000, 0xbfffffff);  // 最多读取到 3072 MB 内存
    struct MEM_MANAGER *memman = (struct MEM_MANAGER*) MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00400000, memory_total - 0x00400000);

    memman_alloc(memman, 12 * 1024 * 1024 + 23333);

    char mem_out_str[200];
    sprintf(mem_out_str, "Total memory: %d MB  Free: %d MB", memory_total / (1024 * 1024), memman_available(memman) / (1024 * 1024));
    put_ascii_str8(binfo->vram, binfo->scrnx, 8, 24, COLOR8_WHITE, mem_out_str);


    // 键盘分配 128 byte 缓冲区
    fifo8_init(&key_buf, 128, key_data);
    // 鼠标分配 512 byte 缓冲区
    fifo8_init(&mouse_buf, 512, mouse_data);

    init_keyboard();
    enable_mouse(&mouse_dec);


    while(1){
        io_cli();  // 处理过程中禁止中断

        if (fifo8_data_available(&key_buf) + fifo8_data_available(&mouse_buf) == 0) {
            io_stihlt();  // 接收中断并等待
        } else {
            int data = fifo8_get(&key_buf);
            if(data != BUFFER_RET_EMPTY) {
                boxfill8(binfo->vram, binfo->scrnx, COLOR8_BLACK, 0, binfo->scrny - 16, binfo->scrnx, binfo->scrny);
                char output[1024];

                sprintf(output, "KEYBOARD: %02X", data);
                put_ascii_str8(binfo->vram, binfo->scrnx, 8, binfo->scrny - 16, COLOR8_WHITE, output);
            }

            data = fifo8_get(&mouse_buf);
            if(data != BUFFER_RET_EMPTY) {
                if(mouse_decode(&mouse_dec, data) != 0) {
                    boxfill8(binfo->vram, binfo->scrnx, COLOR8_BLACK, 0, binfo->scrny - 16, binfo->scrnx, binfo->scrny);
                    char output[1024];

                    sprintf(output, "MOUSE: %d %d %d %d %d", mouse_dec.button & 0x01, (mouse_dec.button & 0x04) / 4, (mouse_dec.button & 0x02) / 2, mouse_dec.x, mouse_dec.y);
                    put_ascii_str8(binfo->vram, binfo->scrnx, 8, binfo->scrny - 16, COLOR8_WHITE, output);
                }
            }
        }
    }
}

