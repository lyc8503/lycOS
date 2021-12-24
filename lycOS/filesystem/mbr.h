#ifndef LYCOS_H_MBR_H
#define LYCOS_H_MBR_H

#include "../bootpack.h"

#define MBR_FILESYSTEM_FAT32 0x0b
#define MBR_FILESYSTEM_FAT32_LBA 0x0c

typedef struct MBR_PARTITION {
    uint8_t active;
    uint8_t head_start;
    uint16_t sector_cylinder_start;
    uint8_t fs;
    uint8_t head_end;
    uint16_t sector_cylinder_end;
    uint32_t lba_start;
    uint32_t sectors;
} MBR_PARTITION;

typedef struct MBR_INFO {
    MBR_PARTITION partitions[4];
} MBR_INFO;

MBR_INFO* init_mbr(uint8_t* mbr_buf);

#endif
