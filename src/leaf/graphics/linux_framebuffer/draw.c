#ifdef GRAPHICS_LINUX_FBDEV

#include <math.h>
#include <stdio.h>

#include "../../draw.h"
#include "../../leaf.h"


void leaf_set_drawing_context(LeafCtx* leaf_ctx) {
    // Nothing to do.
}


// Returns the drawn character width.
float leaf_draw_char
(
    LeafFont* font,
    int pos_x,
    int pos_y,
    char ch
){
    
    int d_x = pos_x;
    int d_y = pos_y;
    
    const int origin_x = d_x;

    for(int y = 0; y < font->real_char_height; y++) {
        
        size_t glyph_index = ch * font->real_char_height + y;
        if(glyph_index >= font->psf_data_size) {
            continue;
        }

        uint8_t glyph_row = font->psf_data[ glyph_index ];


        for(int x = 0; x < font->real_char_width; x++) {
            if(glyph_row & 0x80) {
                p_leaf_set_activefb_pixel_xy
                (
                    d_x, d_y,
                    font->char_color
                );
            }

            glyph_row <<= 1;
            d_x += 1;
        }
        d_y += 1;
        d_x = origin_x;
    }
}

// If leaf_draw_text() 'str_size' is negative, then 'str' must be null terminated.
// Returns the drawn text width.
float leaf_draw_text
(
    LeafFont* font,
    int pos_x,
    int pos_y,
    char* str,
    ssize_t str_size
){
    asm("int3");
    return 0;
}

float leaf_draw_text_fmt
(
    LeafFont* font,
    int pos_x,
    int pos_y,
    const char* fmt,
    ...
){
    return 0;
}

void leaf_draw_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    RGBColor color
){
    int x_beg = (int)floor(pos_x);
    int y_beg = (int)floor(pos_y);
    
    int x_end = x_beg + (int)floor(width);
    int y_end = y_beg + (int)floor(height);

    for(int y = y_beg; y <= y_end; y++) {
        for(int x = x_beg; x <= x_end; x++) {
            p_leaf_set_activefb_pixel_xy(x, y, color);
        }
    }
}


void leaf_draw_texture_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    uint32_t texture,
    RGBColor color,
    int options
){
    // TODO: Implement this.
}



#endif // GRAPHICS_LINUX_FBDEV
