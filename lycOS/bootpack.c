// 启动时保存的一些信息
#include<stdio.h>
#include "bootpack.h"
#include "graphic.h"
#include "dsctbl.h"
#include "int.h"

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

    // 图片显示 HelloWorld!
    put_ascii_str8(binfo->vram, binfo->scrnx, 8, 8, COLOR8_WHITE, "HelloWorld from lycOS!");


    while(1){
        io_hlt();
    }
}
