#ifndef LYCOS_H_FAT32_H
#define LYCOS_H_FAT32_H

#include "../bootpack.h"
#include "../device/pci/drive.h"

typedef struct FAT32_PARTITION {
    DRIVE *drv;
    uint32_t sector_start;
    uint32_t sectors;

    uint8_t sectors_per_cluster;
    uint32_t root_cluster_num;
    uint32_t reserved_sector_count;
    uint32_t first_data_sector;
} FAT32_PARTITION;

FAT32_PARTITION *new_fat32_partition(DRIVE *drv, uint32_t sector_start, uint32_t sectors);

#endif
