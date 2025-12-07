#ifndef LEAF_FONT_H
#define LEAF_FONT_H

#include <stdbool.h>

#define FONT_NUM_CHARS 95


struct glyph_t {
    uint32_t texture; // Texture ID OpenGL can use.
    int width;
    int height;

    int bearing_x;
    int bearing_y;
};

struct font_t {
    struct glyph_t glyphs[FONT_NUM_CHARS];
    float scale;
    
    // Space widht and tab width are in scale
    // if their functions are used to set them.
    float space_width;
    float tab_width;

    // Space and tab width but not in scale.
    float real_space_width;
    float real_tab_width;

    int max_bitmap_width;
    int max_bitmap_height;

    int char_width;
    int char_height;

    // Small space between characters.
    // In scale if leaf_set_font_spacing() is used.
    float spacing; 

    // By default set to 'false'
    bool center_char_to_cell;

    uint32_t shader;
    uint32_t vbo;
    uint32_t vao;
    int shader_color_uniloc; // Uniform locatio for 'font_color'
};


bool leaf_load_font(struct font_t* font, const char* filepath);
void leaf_unload_font(struct font_t* font);

void leaf_set_font_scale(struct font_t* font, float scale);
void leaf_set_font_color(struct font_t* font, float r, float g, float b);
void leaf_set_font_space_width(struct font_t* font, float space_width);
void leaf_set_font_tab_width(struct font_t* font, float tab_width);
void leaf_set_font_spacing(struct font_t* font, float spacing);

void leaf_measure_text
(
    struct font_t* font, 
    int* width_out,
    int* height_out,
    const char* text,
    ssize_t text_length
);


#endif
