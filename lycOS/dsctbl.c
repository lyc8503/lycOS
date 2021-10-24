#include "dsctbl.h"
#include "bootpack.h"


void init_gdtidt() {
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) SEGMENT_DESCRIPTOR_ADDR;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) GATE_DESCRIPTOR_ADDR;

    // 初始化 gdt
    int i;
    for(i = 0; i < 8192; i ++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }

    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);

    load_gdtr(0xffff, SEGMENT_DESCRIPTOR_ADDR);

    // 初始化 idt
    for(i = 0; i < 256; i ++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, GATE_DESCRIPTOR_ADDR);

    return;
}


void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
    if(limit > 0xfffff) {
        ar |= 0x8000;  // G_bit = 1, 使用分页(一页4KB)
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;  // 段上限的低位
    sd->base_low = base & 0xffff;  // 基址(以分页表示, 20位)
    sd->base_mid = (base >> 16) & 0xffff;
    sd->access_permission = ar & 0xff;  // 访问权限
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);  // 段上限的高位和拓展访问权限
    sd->base_high = (base >> 24) & 0xff;

    return;
}


void set_gatedesc(struct GATE_DESCRIPTOR *gd, unsigned int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_permission = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;

    return;
}