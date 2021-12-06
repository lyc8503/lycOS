#include "memory.h"
#include <stdio.h>

struct MEM_MANAGER* sys_memman = (struct MEM_MANAGER*) MEMMAN_ADDR;

// 初始化内存管理器
void memman_init(struct MEM_MANAGER* man) {
    man->free_num = 0;
    man->max_free = 0;
    man->lost_size = 0;
    man->lost_num = 0;
}

// 当前所有可用内存信息
unsigned int memman_available(struct MEM_MANAGER* man) {
    unsigned int i, result = 0;

    for (i = 0; i < man->free_num; i++) {
        result += man->free_info[i].size;
    }
    return result;
}

// 分配内存
unsigned int memman_alloc(struct MEM_MANAGER* man, unsigned int size) {

    unsigned int i, addr;
    for (i = 0; i < man->free_num; i++) {
        if (man->free_info[i].size >= size) {  // 成功找到大小足够的可用空间
            addr = man->free_info[i].start;
            man->free_info[i].start += size;
            man->free_info[i].size -= size;

            if (man->free_info[i].size == 0) {  // size == 0 即回收
                man->free_num--;
                for (; i < man->free_num; i++) {
                    man->free_info[i] = man->free_info[i + 1];  // 把后面的信息前移
                }
            }
            return addr;  // 返回目标内存地址
        }
    }

    return NULL;  // 没有找到可用内存
}

// 释放内存 (添加可用内存信息)
int memman_free(struct MEM_MANAGER* man, unsigned int addr, unsigned int size) {

    int i, j;

    for (i = 0; i < man->free_num; i++) {  // 插入排序(寻找位置)
        if (man->free_info[i].start > addr) {
            break;
        }
    }

    if (i > 0) {  // 找到地址
        if (man->free_info[i - 1].start + man->free_info[i - 1].size == addr) {
            // 与前面的可用内存合并
            man->free_info[i - 1].size += size;

            if (i < man->free_num) {
                if (addr + size == man->free_info[i].start) {  // 与后面合并
                    man->free_info[i - 1].size += man->free_info[i].size;

                    man->free_num--;
                    for (; i < man->free_num; i++) {
                        man->free_info[i] = man->free_info[i + 1];
                    }
                }
            }

            return MEMMAN_FREE_OK;
        }
    }

    // 不能与前面合并
    if (i < man->free_num) {
        if (addr + size == man->free_info[i].start) {  // 与后面合并
            man->free_info[i].start = addr;
            man->free_info[i].size += size;
            return MEMMAN_FREE_OK;
        }
    }

    // 两侧都不能合并 新建一条
    if (man->free_num < MEMMAN_SIZE) {
        // 将 free_info[i] 之后的向后移动一格, 空出一个位置(插入)
        for (j = man->free_num; j > i; j--) {
            man->free_info[j] = man->free_info[j - 1];
        }
        man->free_num++;

        // 记录一下最大值
        man->max_free = man->max_free > man->free_num ? man->max_free : man->free_num;

        man->free_info[i].start = addr;
        man->free_info[i].size = size;
        return MEMMAN_FREE_OK;
    }

    // 失败
    man->lost_num++;
    man->lost_size += size;
    return MEMMAN_FREE_SIZE_NOT_ENOUGH;
}

// 分配以 4kb 为单位的内存
unsigned int memman_alloc_4k(struct MEM_MANAGER *man, unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;  // 向上取整
    return memman_alloc(man, size);
}

unsigned int memman_free_4k(struct MEM_MANAGER *man, unsigned int addr, unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;
    return memman_free(man, addr, size);
}
