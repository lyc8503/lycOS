#include "ahci.h"
#include <string.h>
#include "../../memory/memory.h"
#include "drive.h"

#define trace_ahci(format, args...) do {                    \
    dprintk("[AHCI:%d] " format "\r\n", __LINE__, ##args);  \
} while(0)

// reference: https://wiki.osdev.org/AHCI

typedef enum {
    FIS_TYPE_REG_H2D = 0x27,    // Register FIS - host to device
    FIS_TYPE_REG_D2H = 0x34,    // Register FIS - device to host
    FIS_TYPE_DMA_ACT = 0x39,    // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41,    // DMA setup FIS - bidirectional
    FIS_TYPE_DATA = 0x46,    // Data FIS - bidirectional
    FIS_TYPE_BIST = 0x58,    // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F,    // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS = 0xA1,    // Set device bits FIS - device to host
} FIS_TYPE;

// 从主机发送给设备的命令
typedef struct tagFIS_REG_H2D {
    // DWORD 0
    uint8_t fis_type;    // FIS_TYPE_REG_H2D

    uint8_t pmport: 4;    // Port multiplier
    uint8_t rsv0: 3;        // Reserved
    uint8_t c: 1;        // 1: Command, 0: Control

    uint8_t command;    // Command register
    uint8_t featurel;    // Feature register, 7:0

    // DWORD 1
    uint8_t lba0;        // LBA low register, 7:0
    uint8_t lba1;        // LBA mid register, 15:8
    uint8_t lba2;        // LBA high register, 23:16
    uint8_t device;        // Device register

    // DWORD 2
    uint8_t lba3;        // LBA register, 31:24
    uint8_t lba4;        // LBA register, 39:32
    uint8_t lba5;        // LBA register, 47:40
    uint8_t featureh;    // Feature register, 15:8

    // DWORD 3
    uint8_t countl;        // Count register, 7:0
    uint8_t counth;        // Count register, 15:8
    uint8_t icc;        // Isochronous command completion
    uint8_t control;    // Control register

    // DWORD 4
    uint8_t rsv1[4];    // Reserved
} FIS_REG_H2D;

// 从设备返回的信息
typedef struct tagFIS_REG_D2H {
    // DWORD 0
    uint8_t fis_type;    // FIS_TYPE_REG_D2H

    uint8_t pmport: 4;    // Port multiplier
    uint8_t rsv0: 2;      // Reserved
    uint8_t i: 1;         // Interrupt bit
    uint8_t rsv1: 1;      // Reserved

    uint8_t status;      // Status register
    uint8_t error;       // Error register

    // DWORD 1
    uint8_t lba0;        // LBA low register, 7:0
    uint8_t lba1;        // LBA mid register, 15:8
    uint8_t lba2;        // LBA high register, 23:16
    uint8_t device;      // Device register

    // DWORD 2
    uint8_t lba3;        // LBA register, 31:24
    uint8_t lba4;        // LBA register, 39:32
    uint8_t lba5;        // LBA register, 47:40
    uint8_t rsv2;        // Reserved

    // DWORD 3
    uint8_t countl;      // Count register, 7:0
    uint8_t counth;      // Count register, 15:8
    uint8_t rsv3[2];     // Reserved

    // DWORD 4
    uint8_t rsv4[4];     // Reserved
} FIS_REG_D2H;

// 数据载荷
typedef struct tagFIS_DATA {
    // DWORD 0
    uint8_t fis_type;    // FIS_TYPE_DATA

    uint8_t pmport: 4;    // Port multiplier
    uint8_t rsv0: 4;        // Reserved

    uint8_t rsv1[2];    // Reserved

    // DWORD 1 ~ N
    uint32_t data[1];    // Payload
} FIS_DATA;

// PIO 数据载荷
typedef struct tagFIS_PIO_SETUP {
    // DWORD 0
    uint8_t fis_type;    // FIS_TYPE_PIO_SETUP

    uint8_t pmport: 4;    // Port multiplier
    uint8_t rsv0: 1;        // Reserved
    uint8_t d: 1;        // Data transfer direction, 1 - device to host
    uint8_t i: 1;        // Interrupt bit
    uint8_t rsv1: 1;

    uint8_t status;        // Status register
    uint8_t error;        // Error register

    // DWORD 1
    uint8_t lba0;        // LBA low register, 7:0
    uint8_t lba1;        // LBA mid register, 15:8
    uint8_t lba2;        // LBA high register, 23:16
    uint8_t device;        // Device register

    // DWORD 2
    uint8_t lba3;        // LBA register, 31:24
    uint8_t lba4;        // LBA register, 39:32
    uint8_t lba5;        // LBA register, 47:40
    uint8_t rsv2;        // Reserved

    // DWORD 3
    uint8_t countl;        // Count register, 7:0
    uint8_t counth;        // Count register, 15:8
    uint8_t rsv3;        // Reserved
    uint8_t e_status;    // New value of status register

    // DWORD 4
    uint16_t tc;        // Transfer count
    uint8_t rsv4[2];    // Reserved
} FIS_PIO_SETUP;

// DMA 设置
typedef struct tagFIS_DMA_SETUP {
    // DWORD 0
    uint8_t fis_type;    // FIS_TYPE_DMA_SETUP

    uint8_t pmport: 4;    // Port multiplier
    uint8_t rsv0: 1;        // Reserved
    uint8_t d: 1;        // Data transfer direction, 1 - device to host
    uint8_t i: 1;        // Interrupt bit
    uint8_t a: 1;            // Auto-activate. Specifies if DMA Activate FIS is needed

    uint8_t rsved[2];       // Reserved

    //DWORD 1&2

    uint64_t DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

    //DWORD 3
    uint32_t rsvd;           //More reserved

    //DWORD 4
    uint32_t DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0

    //DWORD 5
    uint32_t TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

    //DWORD 6
    uint32_t resvd;          //Reserved

} FIS_DMA_SETUP;

// HBA FIS 的设置
typedef volatile struct tagHBA_FIS {
    // 0x00
    FIS_DMA_SETUP dsfis;        // DMA Setup FIS
    uint8_t pad0[4];

    // 0x20
    FIS_PIO_SETUP psfis;        // PIO Setup FIS
    uint8_t pad1[12];

    // 0x40
    FIS_REG_D2H rfis;        // Register – Device to Host FIS
    uint8_t pad2[4];

    // 0x58 (TODO: uint16?)
    uint16_t sdbfis;        // Set Device Bit FIS

    // 0x60
    uint8_t ufis[64];

    // 0xA0
    uint8_t rsv[0x100 - 0xA0];
} HBA_FIS;


typedef struct tagHBA_CMD_HEADER {
    // DW0
    uint8_t cfl: 5;        // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a: 1;        // ATAPI
    uint8_t w: 1;        // Write, 1: H2D, 0: D2H
    uint8_t p: 1;        // Prefetchable

    uint8_t r: 1;        // Reset
    uint8_t b: 1;        // BIST
    uint8_t c: 1;        // Clear busy upon R_OK
    uint8_t rsv0: 1;        // Reserved
    uint8_t pmp: 4;        // Port multiplier port

    uint16_t prdtl;        // Physical region descriptor table length in entries

    // DW1
    volatile
    uint32_t prdbc;        // Physical region descriptor byte count transferred

    // DW2, 3
    uint32_t ctba;        // Command table descriptor base address
    uint32_t ctbau;        // Command table descriptor base address upper 32 bits

    // DW4 - 7
    uint32_t rsv1[4];    // Reserved
} HBA_CMD_HEADER;

typedef struct tagHBA_PRDT_ENTRY {
    uint32_t dba;        // Data base address
    uint32_t dbau;        // Data base address upper 32 bits
    uint32_t rsv0;        // Reserved

    // DW3
    uint32_t dbc: 22;        // Byte count, 4M max
    uint32_t rsv1: 9;        // Reserved
    uint32_t i: 1;        // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL {
    // 0x00
    uint8_t cfis[64];    // Command FIS

    // 0x40
    uint8_t acmd[16];    // ATAPI command, 12 or 16 bytes

    // 0x50
    uint8_t rsv[48];    // Reserved

    // 0x80
    HBA_PRDT_ENTRY prdt_entry[1];    // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;


#define    AHCI_BASE    0x10000000  // TODO: ADDR

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

// Start command engine
void start_cmd(HBA_PORT *port) {
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(HBA_PORT *port) {
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;

    // Wait until FR (bit14), CR (bit15) are cleared
    while (1) {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }
}

void port_rebase(HBA_PORT *port, int portno) {
    stop_cmd(port);    // Stop command engine

    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno << 10);
    port->clbu = 0;
    memset((void *) (port->clb), 0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
    port->fbu = 0;
    memset((void *) (port->fb), 0, 256);

    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *) (port->clb);
    int i;
    for (i = 0; i < 32; i++) {
        cmd_header[i].prdtl = 8;    // 8 prdt entries per command table
        // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmd_header[i].ctba = AHCI_BASE + (40 << 10) + (portno << 13) + (i << 8);
        cmd_header[i].ctbau = 0;
        memset((void *) cmd_header[i].ctba, 0, 256);
    }

    start_cmd(port);    // Start command engine
}


#define    SATA_SIG_ATA    0x00000101    // SATA drive
#define    SATA_SIG_ATAPI    0xEB140101    // SATAPI drive
#define    SATA_SIG_SEMB    0xC33C0101    // Enclosure management bridge
#define    SATA_SIG_PM    0x96690101    // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

// Check device type
static int check_type(HBA_PORT *port) {
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT)    // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
}

void probe_port(HBA_MEM *abar, DRIVECTL* ctl) {
    // Search disk in implemented ports
    uint32_t pi = abar->pi;
    int i = 0;
    while (i < 32) {
        if (pi & 1) {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                trace_ahci("SATA drive found at port %d", i);
                add_drive(ctl, AHCI_SATA_DRIVE, &abar->ports[i]);
//                port_rebase(&abar->ports[i], i);
//                trace_ahci("Rebase OK.");
            } else if (dt == AHCI_DEV_SATAPI) {
                trace_ahci("SATAPI drive found at port %d", i);
            } else if (dt == AHCI_DEV_SEMB) {
                trace_ahci("SEMB drive found at port %d", i);
            } else if (dt == AHCI_DEV_PM) {
                trace_ahci("PM drive found at port %d", i);
            } else {
                trace_ahci("No drive found at port %d", i);
            }
        }

        pi >>= 1;
        i++;
    }
}


#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define HBA_PxIS_TFES 0x10

// reference: https://wiki.osdev.org/ATA_Command_Matrix
#define ATA_CMD_READ_DMA_EX 0x25

// Find a free command list slot
int find_cmdslot(HBA_PORT *port) {
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    int i;
    // TODO
    int cmdslots = 32;
    for (i = 0; i < cmdslots; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    trace_ahci("Cannot find free command list entry.");
    return -1;
}

int read_lba48(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf) {
    port->is = (uint32_t) -1;        // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *) port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);    // Command FIS size
    cmdheader->w = 0;        // Read from device
    cmdheader->prdtl = (uint16_t) ((count - 1) >> 4) + 1;    // PRDT entries count

    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *) (cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

    // 8K bytes (16 sectors) per PRDT
    int i;
    for (i = 0; i < cmdheader->prdtl - 1; i++) {
        cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
        cmdtbl->prdt_entry[i].dbc =
                8 * 1024 - 1;    // 8K bytes (this value should always be set to 1 less than the actual value)
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;    // 4K words
        count -= 16;    // 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
    cmdtbl->prdt_entry[i].dbc = (count << 9) - 1;    // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D *) (&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;    // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t) startl;
    cmdfis->lba1 = (uint8_t) (startl >> 8);
    cmdfis->lba2 = (uint8_t) (startl >> 16);
    cmdfis->device = 1 << 6;    // LBA mode

    cmdfis->lba3 = (uint8_t) (startl >> 24);
    cmdfis->lba4 = (uint8_t) starth;
    cmdfis->lba5 = (uint8_t) (starth >> 8);

    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
        spin++;
    }
    if (spin == 1000000) {
        trace_ahci("Port is hung.");
        return 0;
    }

    port->ci = 1 << slot;    // Issue command

    // Wait for completion
    while (1) {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0)
            break;
        if (port->is & HBA_PxIS_TFES)    // Task file error
        {
            trace_ahci("Read disk error.");
            return 0;
        }
    }

    // Check again
    if (port->is & HBA_PxIS_TFES) {
        trace_ahci("Read disk error.");
        return 0;
    }

    return 1;
}

int init_ahci(HBA_MEM *abar, DRIVECTL *ctl) {
    probe_port(abar, ctl);
}
