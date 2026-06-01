/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/
#ifndef LEAF_DRAW_H
#define LEAF_DRAW_H

#include <sys/types.h>

#include "font.h"
#include "color_type.h"


// Returns the drawn character width.
float leaf_draw_char
(
    LeafFont* font,
    int pos_x,
    int pos_y,
    char ch
);

// If leaf_draw_text() 'str_size' is negative, then 'str' must be null terminated.
// Returns the drawn text width.
float leaf_draw_text
(
    LeafFont* font,
    int pos_x,
    int pos_y,
    char* str,
    ssize_t str_size
);

float leaf_draw_text_fmt
(
    LeafFont* font,
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
    RGBColor color
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
    RGBColor color,
    int options
);

void leaf_draw_circle
(
    float pos_x,
    float pos_y,
    float radius,
    int num_triangles,
    RGBColor color
);

#define LEAF_RECT_FADE_HORIZONTAL 0
#define LEAF_RECT_FADE_VERTICAL 1
void leaf_draw_rect_fade
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    RGBColor color_A,
    RGBColor color_B,
    int fade_dir
);


typedef struct LeafCtx_t LeafCtx; // leaf_context
void leaf_set_drawing_context(LeafCtx* leaf_ctx);


#endif
