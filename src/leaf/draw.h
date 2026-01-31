#ifndef LEAF_DRAW_H
#define LEAF_DRAW_H

#include <sys/types.h>

#include "font.h"
#include "color_type.h"


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

float leaf_draw_text_fmt
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    const char* fmt,
    ...
);

void leaf_draw_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    struct color_t color
);


// Options for 'leaf_draw_texture_rect'
#define LEAF_TEXTURE_NO_OPTIONS 0
#define LEAF_TEXTURE_FLIP_Y_ORIGIN (1 << 0)
#define LEAF_TEXTURE_FLIP_X_ORIGIN (1 << 2)
#define LEAF_TEXTURE_FLIP_VERTICAL (1 << 1)
#define LEAF_TEXTURE_FLIP_HORIZONTAL (1 << 3)
void leaf_draw_texture_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    uint32_t texture,
    struct color_t color,
    int options
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
