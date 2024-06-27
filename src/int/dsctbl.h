#ifndef LYCOS_C_DSCTBL_H
#define LYCOS_C_DSCTBL_H

// 以 CPU 资料为基础写成的结构体
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_permission;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_permission;
    short offset_high;
};

#define IDT_ADDR 0x0026f800
#define GDT_ADDR 0x00270000
#define LIMIT_IDT 0x000007ff
#define LIMIT_GDT 0x0000ffff
#define BOOTPACK_ADDR 0x00280000
#define LIMIT_BOOTPACK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

void init_gdtidt();
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, unsigned int offset, int selector, int ar);

#endif
