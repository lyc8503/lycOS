#include "fat32.h"
#include <stdio.h>
#include "../memory/memory.h"

// reference: https://zh.wikipedia.org/wiki/%E6%AA%94%E6%A1%88%E9%85%8D%E7%BD%AE%E8%A1%A8
// reference: https://wiki.osdev.org/FAT

#define trace_fat32(format, args...) do {                       \
    dprintk("[FAT32:%d] " format "\r\n", __LINE__, ##args);     \
} while(0)

#pragma pack(1)
typedef struct BPB {
    uint8_t code[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint16_t directory_entry_count;
    uint16_t sectors_count16;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t lba_start;
    uint32_t large_sectors;
} BPB;

typedef struct EBPB {
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster_num;
    uint16_t fsinfo_sectors;
    uint16_t backup_boot_sectors;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t flag_windows_nt;
    uint8_t signature;
    uint32_t serial;
    uint8_t label[11];
    uint8_t identifier[8];
    uint8_t bootcode[420];
    uint16_t bootable_signature;
} EBPB;

typedef struct FSINFO {
    uint32_t lead_signature;
    uint8_t reserved1[480];
    uint32_t signature;
    uint32_t last_known_free_cluster;
    uint32_t available_cluster_start;
    uint8_t reserved2[12];
    uint32_t trail_signature;
} FSINFO;

typedef struct STANDARD83 {
    uint8_t filename[11];
    uint8_t attrib;
    uint8_t reserved_for_nt;
    uint8_t creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_accessed;
    uint16_t cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t cluster_low;
    uint32_t size;
} STANDARD83;

typedef struct LONG_FILENAME {
    uint8_t order;
    uint8_t character1[10];
    uint8_t attrib;
    uint8_t entry_type;
    uint8_t checksum;
    uint8_t character2[12];
    uint16_t zero;
    uint8_t character3[4];
} LONG_FILENAME;
#pragma pack()


// 读取一个 FAT32 分区
FAT32_PARTITION *new_fat32_partition(DRIVE *drv, uint32_t sector_start, uint32_t sectors) {

    uint8_t *buf = (uint8_t *) memman_alloc_4k(sys_memman, 512);
    if (buf == NULL) return NULL;

    read_drive(drv, buf, sector_start, 1);
    BPB *bpb = (BPB *) buf;
    trace_fat32("fs lba start: %d, count: %d", bpb->lba_start, bpb->large_sectors);
    if (bpb->lba_start != sector_start || bpb->large_sectors != sectors) {
        trace_fat32("lba or sectors not match! not a valid fat32 fs.");
        memman_free_4k(sys_memman, buf, 512);
        return NULL;
    }

    EBPB *ebpb = (EBPB *) (buf + 0x024);
    if (ebpb->signature != 0x28 && ebpb->signature != 0x29) {
        trace_fat32("ebpb signature not match!");
        memman_free_4k(sys_memman, buf, 512);
        return NULL;
    }


    FAT32_PARTITION *partition = memman_alloc_4k(sys_memman, sizeof(FAT32_PARTITION));
    if (partition == NULL) {
        memman_free_4k(sys_memman, buf, 512);
        return NULL;
    }

    trace_fat32(
            "sectors_per_cluster: %d, root_cluster_num: %d, reserved_sector_count: %d, fat count: %d, sectors per fat: %d",
            bpb->sectors_per_cluster,
            ebpb->root_cluster_num,
            bpb->reserved_sector_count,
            bpb->fat_count,
            ebpb->sectors_per_fat);

    partition->sectors_per_cluster = bpb->sectors_per_cluster;
    partition->root_cluster_num = ebpb->root_cluster_num;
    partition->reserved_sector_count = bpb->reserved_sector_count;
    partition->first_data_sector = bpb->reserved_sector_count + (bpb->fat_count * ebpb->sectors_per_fat);

    memman_free_4k(sys_memman, buf, 512);

    partition->drv = drv;
    partition->sector_start = sector_start;
    partition->sectors = sectors;

    iterate_fat(partition);
    list_directories(partition);

    return partition;
}

uint32_t get_cluster_start(FAT32_PARTITION *partition, int cluster) {
    return ((cluster - 2) * partition->sectors_per_cluster) + partition->first_data_sector;
}

void iterate_fat(FAT32_PARTITION *partition) {
//    int start_sector = partition->reserved_sector_count + partition->sector_start;
//    uint8_t *buf = (uint8_t *) memman_alloc_4k(sys_memman, 512 * 8);
//    if (buf == NULL) {
//        // TODO
//    }
//
//    read_drive(partition->drv, buf, start_sector, 8);
//
//    int i;
//
//    uint32_t *fat = buf;
//
//    for (i = 512 * 2 / 8; i < 512; i++) {
//        printk("%08x\n", *(fat + i));
//    }

}

void list_directories(FAT32_PARTITION *partition) {
    int start_sector = get_cluster_start(partition, partition->root_cluster_num) + partition->sector_start;
    uint8_t *buf = (uint8_t *) memman_alloc_4k(sys_memman, 512 * partition->sectors_per_cluster);

    read_drive(partition->drv, buf, start_sector, partition->sectors_per_cluster);

    int i;
    for (i = 0; i < 512 * partition->sectors_per_cluster; i += 32) {
        int j;
        for (j = 0; j < 32; j++) {
            printk("%02x ", buf[i + j]);
        }
        printk("\r\n");

        STANDARD83 *entry = (STANDARD83 *) (buf + i);

        if ((entry->attrib & 0x0F) == 0x0F) {
            trace_fat32("Long file name.");
        }

        if (entry->attrib & 0x10) {
            trace_fat32("Directory.");
        }

        // Archive bit is ignored here.

        printk("%s", entry->filename);
        printk("\r\n");
    }
}
