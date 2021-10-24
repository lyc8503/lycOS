// 启动时保存的一些信息
#include<stdio.h>
#include "bootpack.h"
#include "graphic.h"

// 系统入口
void MyOSMain() {

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
