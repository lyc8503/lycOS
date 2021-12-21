#include "pci.h"
#include <stdio.h>
#include "../../bootpack.h"
#include "../serial.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// reference: https://wiki.osdev.org/PCI

// 读取特定 PCI 设备的配置, 不存在返回 0xFFFF, offset % 2 应该 == 0
unsigned short pci_config_read16(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    ASSERT(offset % 2 == 0);

    unsigned int address;
    unsigned int lbus  = bus;
    unsigned int lslot = slot;
    unsigned int lfunc = func;
    unsigned short tmp;

    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out the address
    io_out32(PCI_CONFIG_ADDRESS, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (unsigned short)(((unsigned int)io_in32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

unsigned short get_vendor_id(unsigned char bus, unsigned char slot, unsigned char func) {
    return pci_config_read16(bus, slot, func, 0x0);
}

unsigned char get_header_type(unsigned char bus, unsigned char slot, unsigned char func) {
    return pci_config_read16(bus, slot, func, 0xE) & 0xFF;
}

void check_function(unsigned char bus, unsigned char device, unsigned char function) {
    char temp[100];
    sprintf(temp, "bus: %d, dev: %d, func: %d\r\n", bus, device, function);
    write_serial_str(temp);

//    switch (get_header_type(bus, device, function) & 0xF) {
//        case 0x0:
//            write_serial_str("- Normal PCI device\r\n");
//            break;
//        case 0x1:
//            write_serial_str("- PCI-to-PCI bridge\r\n");
//            break;
//        case 0x2:
//            write_serial_str("- PCI-to-CardBus bridge\r\n");
//            break;
//    }

    ASSERT(get_header_type(bus, device, function) & 0xF == 0x0);

    sprintf(temp, "class code / sub: %x\r\n", pci_config_read16(bus, device, function, 0xA));
    write_serial_str(temp);

    // IDE
    if (pci_config_read16(bus, device, function, 0xA) == 0x0101) {
        write_serial_str("IDE device found!\r\n");

        sprintf(temp, "ProgIF: %x\r\n", pci_config_read16(bus, device, function, 0x8) >> 8);
        write_serial_str(temp);
    }
}

void check_device(unsigned char bus, unsigned char device) {
    unsigned char function = 0;

    unsigned short vendor_id = get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) {
//      Device doesn't exist
        return;
    }

    // TODO: Clion 为什么会警告...
    check_function(bus, device, function);
    unsigned char header_type = get_header_type(bus, device, function);
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
    unsigned short bus;
    unsigned char device;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            check_device(bus, device);
        }
    }
}

void init_pci() {
    check_all_buses();
}
