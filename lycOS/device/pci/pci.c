#include "pci.h"
#include <stdio.h>
#include "../../bootpack.h"
#include "ahci.h"


#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// reference: https://wiki.osdev.org/PCI

// 读取特定 PCI 设备的配置, 不存在返回 0xFFFF, offset % 2 应该 == 0
uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    ASSERT(offset % 2 == 0);

    uint32_t address;
    uint32_t lbus  = bus;
    uint32_t lslot = slot;
    uint32_t lfunc = func;
    uint16_t tmp;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    io_out32(PCI_CONFIG_ADDRESS, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)(((uint32_t)io_in32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint16_t get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_config_read16(bus, slot, func, 0x0);
}

uint8_t get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_config_read16(bus, slot, func, 0xE) & 0xFF;
}

void check_function(uint8_t bus, uint8_t device, uint8_t function) {
    printk("bus: %d, dev: %d, func: %d\r\n", bus, device, function);

//    switch (get_header_type(bus, device, function) & 0xF) {
//        case 0x0:
//            printk("- Normal PCI device\r\n");
//            break;
//        case 0x1:
//            printk("- PCI-to-PCI bridge\r\n");
//            break;
//        case 0x2:
//            printk("- PCI-to-CardBus bridge\r\n");
//            break;
//    }

    ASSERT((get_header_type(bus, device, function) & 0xF) == 0x0);

    printk("class code / sub: %x\r\n", pci_config_read16(bus, device, function, 0xA));

    // AHCI
    if (pci_config_read16(bus, device, function, 0xA) == 0x0106) {
        printk("AHCI device found!\r\n");
        uint32_t bar5 = (((uint32_t) pci_config_read16(bus, device, function, 0x26) << 16) +
                pci_config_read16(bus, device, function, 0x24)) & 0xFFFFFFF0;

        printk("bar5 addr: %p\r\n", bar5);

        printk("command: %p\r\n", pci_config_read16(bus, device, function, 0x4));

        init_ahci(bar5);

    }
}

void check_device(uint8_t bus, uint8_t device) {
    uint8_t function = 0;

    uint16_t vendor_id = get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) {
//      Device doesn't exist
        return;
    }

    check_function(bus, device, function);
    uint8_t header_type = get_header_type(bus, device, function);
    if((header_type & 0x80) != 0) {
        // It's a multi-function device, so check remaining functions
        for (function = 1; function < 8; function++) {
            if (get_vendor_id(bus, device, function) != 0xFFFF) {
                check_function(bus, device, function);
            }
        }
    }
}

void check_all_buses() {
    uint16_t bus;
    uint8_t device;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            check_device(bus, device);
        }
    }
}

void init_pci() {
    check_all_buses();
}
