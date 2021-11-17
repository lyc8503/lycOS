#ifndef LYCOS_C_LAYER_H
#define LYCOS_C_LAYER_H

#define LAYER_TRANSPARENT 255
#define MAX_LAYERS 512

#include "../bootpack.h"

struct LAYER {
    int loc_x, loc_y, width, height, priority, flags;  // 其中 priority 的小的图层在下
    char* content;
};

struct LAYERCTL {
    struct LAYER* layers[MAX_LAYERS];
    int layer_num, max_x, max_y;
    char* buf;
};

void init_layerctl(struct LAYERCTL* ctl, int max_x, int max_y);
void layerctl_draw(struct LAYERCTL* ctl, char* target);
struct LAYER* alloc_layer(struct LAYERCTL* ctl, int width, int height, int loc_x, int loc_y);

#endif
