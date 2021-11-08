// 共计 32768 - 200 个 FREE_INFO, 大小 < 256KB
#define MEMMAN_SIZE 32568
#define MEMMAN_ADDR 0x003c0000

#define MEMMAN_OOM 0x00000000
#define MEMMAN_FREE_OK 0
#define MEMMAN_FREE_SIZE_NOT_ENOUGH -1


struct FREE_INFO {
    unsigned int start, size;  // 一条可用空间信息
};

struct MEM_MANAGER {
    struct FREE_INFO free_info[MEMMAN_SIZE];  // 所有的可用信息

    int free_num, max_free, lost_size, lost_num;  // max_frees 是 frees 达到过的最大值
};


void memman_init(struct MEM_MANAGER* man);
unsigned int memman_available(struct MEM_MANAGER* man);
unsigned int memman_alloc(struct MEM_MANAGER* man, unsigned int size);
int memman_free(struct MEM_MANAGER* man, unsigned int addr, unsigned int size);

