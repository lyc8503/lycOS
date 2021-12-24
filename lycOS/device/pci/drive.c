#include "drive.h"
#include "../../memory/memory.h"
#include "string.h"
#include "ahci.h"

#define trace_drv(format, args...) do {                         \
    dprintk("[DRIVE:%d] " format "\r\n", __LINE__, ##args);     \
} while(0)


DRIVECTL *init_drivectl(int max) {
    DRIVECTL *ctl = (DRIVECTL *) memman_alloc_4k(sys_memman, sizeof(DRIVECTL));
    memset(ctl, 0, sizeof(DRIVECTL));
    ctl->drives = (DRIVE *) memman_alloc_4k(sys_memman, sizeof(DRIVE) * max);
    ctl->max = max;
    return ctl;
}

// 注册硬盘
int add_drive(DRIVECTL *ctl, enum DRIVE_TYPE type, void *port) {
    ASSERT(ctl->count != ctl->max);

    trace_drv("New drive registered: %d", ctl->count);

    ctl->drives[ctl->count].port = port;
    ctl->drives[ctl->count].id = ctl->count;
    ctl->drives[ctl->count++].type = type;

    return ctl->count - 1;
}

// 从指定的磁盘 ID 读取 n byte 的数据
void read_drive(DRIVE *drv, uint8_t *buf, uint32_t start, uint32_t sectors) {
    switch (drv->type) {
        case AHCI_SATA_DRIVE:
            read_lba48(drv->port, start, 0, sectors * 512 / 2, buf);
            break;
        default:
            ASSERT(0);
    }
}
