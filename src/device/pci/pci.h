#ifndef LYCOS_C_PCI_H
#define LYCOS_C_PCI_H

#include "../../bootpack.h"

#define MAX_PCI_DEVICE_FUNCS 65536

#define PCI_DEV_AHCI_CTL 0x0106

typedef struct PCI_DEVICE_FUNC {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} PCI_DEVICE_FUNC;

typedef struct PCICTL {
    PCI_DEVICE_FUNC functions[MAX_PCI_DEVICE_FUNCS];
    int count;
} PCICTL;

PCICTL *init_pci();
uint16_t get_class_and_subclass(PCI_DEVICE_FUNC *devf);
uint32_t get_bar(PCI_DEVICE_FUNC *devf, int index);

#endif
