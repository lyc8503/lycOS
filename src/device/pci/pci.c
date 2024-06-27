#include "pci.h"
#include "../../bootpack.h"
#include <stdio.h>
#include <string.h>
#include "ahci.h"
#include "../../memory/memory.h"


#define trace_pci(format, args...) do {                     \
    dprintk("[PCI:%d] " format "\r\n", __LINE__, ##args);   \
} while(0)

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// reference: https://wiki.osdev.org/PCI

// 读取特定 PCI 设备的配置, 不存在返回 0xFFFF, offset % 2 应该 == 0
uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    ASSERT(offset % 2 == 0);

    uint32_t address;
    uint32_t lbus = bus;
    uint32_t lslot = slot;
    uint32_t lfunc = func;
    uint16_t tmp;

    // Create configuration address as per Figure 1
    address = (uint32_t) ((lbus << 16) | (lslot << 11) |
                          (lfunc << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    // Write out the address
    io_out32(PCI_CONFIG_ADDRESS, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t) (((uint32_t) io_in32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint16_t get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_config_read16(bus, slot, func, 0x0);
}

uint8_t get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_config_read16(bus, slot, func, 0xE) & 0xFF;
}


uint16_t get_class_and_subclass(PCI_DEVICE_FUNC *devf) {
//    switch (get_header_type(bus, device, function) & 0xF) {
//        case 0x0:
//            trace_pci("- Normal PCI device");
//            break;
//        case 0x1:
//            trace_pci("- PCI-to-PCI bridge");
//            break;
//        case 0x2:
//            trace_pci("- PCI-to-CardBus bridge");
//            break;
//    }

    ASSERT((get_header_type(devf->bus, devf->device, devf->function) & 0xF) == 0x0);
    return pci_config_read16(devf->bus, devf->device, devf->function, 0xA);
}

uint32_t get_bar(PCI_DEVICE_FUNC *devf, int index) {
    return (((uint32_t) pci_config_read16(devf->bus, devf->device, devf->function, 0x10 + index * 4 + 2) << 16) +
            pci_config_read16(devf->bus, devf->device, devf->function, 0x10 + index * 4)) & 0xFFFFFFF0;
}


uint8_t check_device_functions(uint8_t bus, uint8_t device) {
    uint8_t function = 0;

    uint8_t ret = 0;

    uint16_t vendor_id = get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) {
//      Device doesn't exist
        return ret;
    }

    ret |= 0x01;

    uint8_t header_type = get_header_type(bus, device, function);
    if ((header_type & 0x80) != 0) {
        // It's a multi-function device, so check remaining functions
        for (function = 1; function < 8; function++) {
            if (get_vendor_id(bus, device, function) != 0xFFFF) {
                ret |= (0x01 << function);
            }
        }
    }

    return ret;
}

void check_all_buses(PCICTL *ctl) {
    uint16_t bus;
    uint8_t device;

    // 遍历所有 bus 和 device
    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            uint8_t functions = check_device_functions(bus, device);

            // 如果有功能
            if (functions != 0) {
                int func;
                for (func = 0; func < 8; func++) {

                    // 添加所有功能到 pci_ctl
                    if (functions & (0x01 << func)) {
                        trace_pci("pci device function found: bus: %d, dev: %d, func: %d", bus, device, func);
                        ctl->functions[ctl->count].bus = bus;
                        ctl->functions[ctl->count].device = device;
                        ctl->functions[ctl->count++].function = func;
                    }
                }
            }
        }
    }
}

PCICTL *init_pci() {
    PCICTL *ctl = (PCICTL *) memman_alloc_4k(sys_memman, sizeof(PCICTL));
    if (ctl == NULL) return NULL;
    memset(ctl, 0, sizeof(PCICTL));

    check_all_buses(ctl);
    return ctl;
}
