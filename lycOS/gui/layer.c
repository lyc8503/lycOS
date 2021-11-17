#include "layer.h"
#include <stdio.h>
#include <string.h>
#include "../memory/memory.h"


// 初始化层管理器
void init_layerctl(struct LAYERCTL* ctl, int max_x, int max_y) {
    ctl->layer_num = 0;
    ctl->max_x = max_x;
    ctl->max_y = max_y;

    // 输出缓冲区
    ctl->buf = (char*) memman_alloc(sys_memman, max_x * max_y * sizeof(char));
}

void layerctl_draw(struct LAYERCTL* ctl, char* target) {
    int i, x, y;
    for (i = 0; i < ctl->layer_num; i++) {
        for (x = 0; x < ctl->max_x; x++){
            for (y = 0; y < ctl->max_y; y++) {

            }
        }
    }
}

// 分配出一个空层
struct LAYER* alloc_layer(struct LAYERCTL* ctl, int width, int height, int loc_x, int loc_y) {
    // TODO 检查图层大小和 ctl 大小

    // 图层满了
    if (ctl->layer_num >= MAX_LAYERS) {
        return NULL;
    }

    struct LAYER* layer = (struct LAYER*) memman_alloc(sys_memman, sizeof(struct LAYER));

    layer->loc_x = loc_x;
    layer->loc_y = loc_y;
    layer->width = width;
    layer->height = height;

    layer->priority = ctl->layer_num;
    ctl->layers[ctl->layer_num++] = layer;

    layer->content = (char*) memman_alloc(sys_memman, sizeof(char) * width * height);
    memset(layer->content, LAYER_TRANSPARENT, sizeof(char) * width * height);

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
