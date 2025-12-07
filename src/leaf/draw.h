#ifndef LEAF_DRAW_H
#define LEAF_DRAW_H

#include <sys/types.h>

#include "font.h"


struct color_t { // 0 - 255
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Returns the drawn character width.
float leaf_draw_char
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    char ch
);

// If leaf_draw_text() 'str_size' is negative, then 'str' must be null terminated.
// Returns the drawn text width.
float leaf_draw_text
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    char* str,
    ssize_t str_size
);

void leaf_draw_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    struct color_t color
);

void leaf_draw_circle
(
    float pos_x,
    float pos_y,
    float radius,
    int num_triangles,
    struct color_t color
);

#define LEAF_RECT_FADE_HORIZONTAL 0
#define LEAF_RECT_FADE_VERTICAL 1
void leaf_draw_rect_fade
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    struct color_t color_A,
    struct color_t color_B,
    int fade_dir
);


struct leaf_ctx_t; // leaf_context
void leaf_set_drawing_context(struct leaf_ctx_t* leaf_ctx);


#endif
