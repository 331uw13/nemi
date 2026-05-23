#ifndef LEAF_FONT_H
#define LEAF_FONT_H

#include <stdbool.h>

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
    LeafFontGlyph glyphs[FONT_NUM_CHARS];
    bool loaded;
    
    // By default set to 'false'
    bool center_char_to_cell;
    

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
    //int shader_color_uniloc; // Uniform locatio for 'font_color'
}
LeafFont;

struct leaf_ctx_t;
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
