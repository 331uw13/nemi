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


#ifdef GRAPHICS_OPENGL
#define FONT_NUM_CHARS 95

typedef struct LeafFontGlyph_t {
    int width;
    int height;

    int bearing_x;
    int bearing_y;

    int atlas_x;
}
LeafFontGlyph;
#endif // GRAPHICS_OPENGL

#ifdef GRAPHICS_LINUX_FBDEV

typedef struct PSF1_Header_t {
    uint8_t magic_bytes[2];
    uint8_t mode;
    uint8_t char_size;
}
PSF1_Header;

typedef struct PSF2_Header_t {
    uint8_t  magic_bytes[4];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t length;
    uint32_t char_size;
    uint32_t height;
    uint32_t width;
}
PSF2_Header;

typedef enum PSFVersion_e {
    PSF_VERSION_1,
    PSF_VERSION_2
}
PSFVersion;

#endif // GRAPHICS_LINUX_FBDEV


typedef struct LeafFont_t {

    float scale;
    
    // Space widht and tab width are in scale
    // if their functions are used to set them.
    float space_width;
    float tab_width;

    // Space and tab width but not in scale.
    float real_space_width;
    float real_tab_width;

    uint32_t max_bitmap_width;
    uint32_t max_bitmap_height;

    int char_width;
    int char_height;

    float italic;

    // Small space between characters.
    // In scale if leaf_set_font_spacing() is used.
    float spacing; 

    
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

#ifdef GRAPHICS_LINUX_FBDEV

    PSFVersion psf_version;
    union {
        PSF1_Header psf1;
        PSF2_Header psf2;
    }
    psf_header;

    uint8_t* psf_data;
    size_t   psf_data_size;

#endif // GRAPHICS_LINUX_FBDEV

    bool loaded;
    
    // By default set to 'false'
    bool center_char_to_cell;
}
LeafFont;

//struct leaf_ctx_t;

bool leaf_load_font(LeafFont* font, const char* filepath);
void leaf_unload_font(LeafFont* font);

void leaf_set_font_scale        (LeafFont* font, float scale);
void leaf_set_font_color        (LeafFont* font, RGBColor color);
void leaf_set_font_space_width  (LeafFont* font, float space_width);
void leaf_set_font_tab_width    (LeafFont* font, float tab_width);
void leaf_set_font_spacing      (LeafFont* font, float spacing);

void leaf_measure_text
(
    LeafFont* font, 
    int* width_out,
    int* height_out,
    const char* text,
    ssize_t text_length
);

void leaf_font_render(LeafFont* font);

#endif
