#include "dsctbl.h"
#include "../bootpack.h"


void init_gdtidt() {
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) GDT_ADDR;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) IDT_ADDR;

    // 初始化 gdt
    int i;
    for(i = 0; i <= LIMIT_GDT / 8; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }

    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_BOOTPACK, BOOTPACK_ADDR, AR_CODE32_ER);

    load_gdtr(LIMIT_GDT, GDT_ADDR);

    // 初始化 idt
    for(i = 0; i <= LIMIT_IDT / 8; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }

    set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 << 3, AR_INTGATE32);

    load_idtr(LIMIT_IDT, IDT_ADDR);
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
}


void set_gatedesc(struct GATE_DESCRIPTOR *gd, unsigned int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_permission = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
}
