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

#ifndef LEAF_FONT_H
#define LEAF_FONT_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "color_type.h"


#define FONT_NUM_CHARS 95

typedef struct LeafFontGlyph_t {
    int width;
    int height;

    int bearing_x;
    int bearing_y;

    int atlas_x;
}
LeafFontGlyph;

typedef struct LeafFont_t {

    float scale;
    
    // NOTE:
    // Do not modify the variables related
    // to character size and spacing directly.
    // They in scale only if their designated functions are used.

    float space_width;
    float tab_width;

    // Character size in scale.
    int char_width;
    int char_height;

    float italic; // TODO: Rename.

    // Small space between characters.
    float spacing; 

    // Space and tab width (Not in scale!)
    float real_space_width;
    float real_tab_width;
   
    // Character size  (Not in scale!)
    uint32_t real_char_width;
    uint32_t real_char_height;

    
#ifdef GRAPHICS_OPENGL
    LeafFontGlyph glyphs[FONT_NUM_CHARS];
    uint32_t texture;
    uint32_t texture_width;
    uint32_t texture_height;
    uint32_t shader;
    uint32_t vbo;
    uint32_t vao;
    size_t   vbo_memsize;
    size_t   vbo_data_offset;
    size_t   vbo_num_vertices;
    float    char_color_r;
    float    char_color_g;
    float    char_color_b;
#endif // GRAPHICS_OPENGL


    bool loaded;
    
    // By default set to 'false'
    bool center_char_to_cell;
}
LeafFont;



bool leaf_load_font(LeafFont* font, const char* filepath);
void leaf_unload_font(LeafFont* font);


// OpenGL specific.
void leaf_font_render(LeafFont* font);



// See 'font_common.c' for implementations:

void leaf_set_font_scale        (LeafFont* font, float scale);
void leaf_set_font_color        (LeafFont* font, RGBColor color);
void leaf_set_font_space_width  (LeafFont* font, float space_width);
void leaf_set_font_tab_width    (LeafFont* font, float tab_width);
void leaf_set_font_spacing      (LeafFont* font, float spacing);

/*
void leaf_measure_text
(
    LeafFont* font, 
    int* width_out,
    int* height_out,
    const char* text,
    ssize_t text_length
);
*/


#endif
