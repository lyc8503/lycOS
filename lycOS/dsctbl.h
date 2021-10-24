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

#define GATE_DESCRIPTOR_ADDR 0x0026f800
#define SEGMENT_DESCRIPTOR_ADDR 0x00270000

void init_gdtidt();
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, unsigned int offset, int selector, int ar);
