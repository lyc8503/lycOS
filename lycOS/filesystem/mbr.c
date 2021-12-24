#include "mbr.h"
#include "../memory/memory.h"
#include <stdio.h>
#include <string.h>

#define trace_mbr(format, args...) do {                     \
    dprintk("[MBR:%d] " format "\r\n", __LINE__, ##args);   \
} while(0)

// reference: https://en.wikipedia.org/wiki/Master_boot_record
// reference: https://wiki.osdev.org/Partition_Table

MBR_INFO* init_mbr(uint8_t* mbr_buf) {

    // 判断是否为 MBR
    if (mbr_buf[510] != 0x55 || mbr_buf[511] != 0xaa) {
        trace_mbr("not a valid mbr.");
        return NULL;
    }

    MBR_INFO *info = (MBR_INFO*) memman_alloc_4k(sys_memman, sizeof(MBR_INFO));
    memcpy(info, mbr_buf + 0x01BE, 64);
    
    int i;
    for (i = 0; i < 4; i++) {
        trace_mbr("fs: %02x lba: %d, sectors: %d",
               info->partitions[i].fs,
               info->partitions[i].lba_start,
               info->partitions[i].sectors);
    }

    return info;
}