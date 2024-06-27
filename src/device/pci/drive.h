#ifndef LYCOS_H_DRIVE_H
#define LYCOS_H_DRIVE_H

#include "../../bootpack.h"

enum DRIVE_TYPE {
    AHCI_SATA_DRIVE
};

typedef struct DRIVE {
    enum DRIVE_TYPE type;
    int id;

    void *port;
} DRIVE;

typedef struct DRIVECTL {
    DRIVE *drives;
    int count;
    int max;
} DRIVECTL;


DRIVECTL *init_drivectl(int max);

int add_drive(DRIVECTL *ctl, enum DRIVE_TYPE type, void *port);

void read_drive(DRIVE *drv, uint8_t *buf, uint32_t start, uint32_t sectors);


#endif
