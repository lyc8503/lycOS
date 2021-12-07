#include "layer.h"
#include <stdio.h>
#include <string.h>
#include "../memory/memory.h"
#include "graphic.h"


// 初始化层管理器
void init_layerctl(struct LAYERCTL* ctl, int max_x, int max_y) {
    ctl->layer_num = 0;
    ctl->max_x = max_x;
    ctl->max_y = max_y;

    // 输出缓冲区
    ctl->buf = (unsigned char*) memman_alloc_4k(sys_memman, max_x * max_y * sizeof(unsigned char));
}

void layerctl_draw(struct LAYERCTL* ctl, unsigned char* target) {
    int i, x, y;

    // TODO: 效率优化
    for (i = 0; i < 2; i++) {
        struct LAYER *layer = ctl->layers[i];
        for (x = 0; x < layer->width; x++){
            for (y = 0; y < layer->height; y++) {
                if (*(layer->content + layer->width * y + x) != LAYER_TRANSPARENT &&
                    layer->loc_y + y < ctl->max_y &&
                    layer->loc_x + x < ctl->max_x &&
                    layer->loc_x + x >= 0 &&
                    layer->loc_y + y >= 0) {
                    *(ctl->buf + (layer->loc_y + y) * ctl->max_x + layer->loc_x + x) = *(layer->content + layer->width * y + x);
                }
            }
        }
    }

    memcpy(target, ctl->buf, ctl->max_x * ctl->max_y * sizeof(unsigned char));
}

// 分配出一个空层
struct LAYER* alloc_layer(struct LAYERCTL* ctl, int width, int height, int loc_x, int loc_y) {

    // 图层满了
    if (ctl->layer_num >= MAX_LAYERS) {
        return NULL;
    }

    struct LAYER* layer = (struct LAYER*) memman_alloc(sys_memman, sizeof(struct LAYER));

    // 内存分配失败
    if (layer == NULL) {
        return NULL;
    }

    layer->loc_x = loc_x;
    layer->loc_y = loc_y;
    layer->width = width;
    layer->height = height;
    layer->content = (unsigned char*) memman_alloc_4k(sys_memman, sizeof(unsigned char) * width * height);

    if (layer->content == NULL) {
        memman_free_4k(sys_memman, (int) layer, sizeof(struct LAYER));
        return NULL;
    }

    memset(layer->content, LAYER_TRANSPARENT, sizeof(unsigned char) * width * height);

    layer->priority = ctl->layer_num++;
    ctl->layers[layer->priority] = layer;

    return layer;
}

// 回收一个空层
void release_layer(struct LAYER* layer){

//    layer->loc_x =
//
//    layer->set_size(0, 0);
//    layer->set_location(0, 0);
//    layer->inuse_flag = 0;

}
