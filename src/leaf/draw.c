#include <GL/glew.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include <stdio.h> // temporary.

#include "draw.h"
#include "leaf.h"
#include "shaders.h"

static struct leaf_ctx_t* g_leaf_ctx = NULL;

void leaf_set_drawing_context(struct leaf_ctx_t* leaf_ctx) {
    g_leaf_ctx = leaf_ctx;
}

void leaf_normalize_coords(float x_in, float y_in, float* x_out, float* y_out) {
    y_in = g_leaf_ctx->win_height - y_in;
    *x_out = (x_in / (float)g_leaf_ctx->win_width) * 2.0f - 1.0f;
    *y_out = (y_in / (float)g_leaf_ctx->win_height) * 2.0f - 1.0f;
}

float leaf_draw_char
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    char chr
){
    if((chr < 0x20) || (chr > 0x7E)) {
        return 0.0f; // Not ascii character.
    }

    struct glyph_t* glyph = &font->glyphs[chr - 0x20];

   // float ch_width = glyph->width * font->scale;
   // float ch_height = glyph->height * font->scale;

    float ch_width = (float)font->max_bitmap_width * font->scale;
    float ch_height = (float)font->max_bitmap_height * font->scale;

    float cw = ch_width / (float)g_leaf_ctx->win_width;
    float ch = ch_height / (float)g_leaf_ctx->win_height;
    
  
    pos_y = g_leaf_ctx->win_height - pos_y;
    float x = (pos_x / (float)g_leaf_ctx->win_width) * 2.0f - 1.0f;
    float y = (pos_y / (float)g_leaf_ctx->win_height) * 2.0f - 1.0f;
    

    float glyph_vertices[] = {
        x,    y-ch, 0.0f, 1.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic,  
        x,    y,    0.0f, 0.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic,  
        x+cw, y,    1.0f, 0.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic, 
        x,    y-ch, 0.0f, 1.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic, 
        x+cw, y,    1.0f, 0.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic, 
        x+cw, y-ch, 1.0f, 1.0f, glyph->atlas_x, font->char_color_r, font->char_color_g, font->char_color_b, y, font->italic
    };

    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferSubData(
            GL_ARRAY_BUFFER,
            font->vbo_data_offset,
            sizeof(glyph_vertices), glyph_vertices);

    font->vbo_data_offset += sizeof(glyph_vertices);
    font->vbo_num_vertices += 6;
    
    

    float ret_width = 0.0f;
    if(!font->center_char_to_cell) {
        ret_width = ch_width / 2.0f;
    }
    else {
        ret_width = (float)font->char_width;
    }

    return ret_width;
}


float leaf_draw_text
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    char* str,
    ssize_t str_size
){
    float total_width = 0.0f;

    if(str_size < 0) {
        str_size = strlen(str);
    }

    for(ssize_t i = 0; i < str_size; i++) {
        char ch = str[i];
        if(ch == 0x20/*space*/) {
            pos_x += font->space_width;
            total_width += font->space_width;
            continue;
        }
        else
        if(ch == 0x09/*horizontal tab*/) {
            pos_x += font->tab_width;
            total_width += font->tab_width;
            continue;
        }

        float ch_width = leaf_draw_char(font, pos_x, pos_y, ch);
        ch_width += font->spacing;

        pos_x += ch_width;
        total_width += ch_width;
    }

    return total_width;
}

float leaf_draw_text_fmt
(
    struct font_t* font,
    int pos_x,
    int pos_y,
    const char* fmt,
    ...
){
    float retv = 0.0f;
    va_list args;
    va_start(args, fmt);

    char buffer[256] = { 0 };
    ssize_t len = vsnprintf(buffer, sizeof(buffer)-1, fmt, args);
        
    if(len > 0) {
        retv = leaf_draw_text(font, pos_x, pos_y, buffer, len);
    }
    va_end(args);
    return retv;
}


void leaf_draw_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    struct color_t color
){
    pos_y = g_leaf_ctx->win_height - pos_y;
    float x = (pos_x / (float)g_leaf_ctx->win_width) * 2.0f - 1.0f;
    float y = (pos_y / (float)g_leaf_ctx->win_height) * 2.0f - 1.0f;

    float w = width / ((float)g_leaf_ctx->win_width / 2.0f);
    float h = height / ((float)g_leaf_ctx->win_height / 2.0f);

    float r = (float)color.r / 255.0f;
    float g = (float)color.g / 255.0f;
    float b = (float)color.b / 255.0f;

    float vertices[] = {
        x,   y-h, r, g, b,
        x,   y,   r, g, b,
        x+w, y,   r, g, b,

        x,   y-h, r, g, b,
        x+w, y,   r, g, b,
        x+w, y-h, r, g, b
    };

    leaf_render_vertices(g_leaf_ctx, vertices, sizeof(vertices));
}

void leaf_draw_texture_rect
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    uint32_t texture,
    struct color_t color,
    int options
){
    if(options & LEAF_TEXTURE_FLIP_Y_ORIGIN) {
        pos_y = g_leaf_ctx->win_height - pos_y;
        pos_y -= height;
    }

    if(options & LEAF_TEXTURE_FLIP_X_ORIGIN) {
        pos_x = g_leaf_ctx->win_width - pos_x;
        pos_x -= width;
    }

    float x = (pos_x / (float)g_leaf_ctx->win_width) * 2.0f - 1.0f;
    float y = (pos_y / (float)g_leaf_ctx->win_height) * 2.0f - 1.0f;

    float w = width / ((float)g_leaf_ctx->win_width / 2.0f);
    float h = height / ((float)g_leaf_ctx->win_height / 2.0f);

    float r = (float)color.r / 255.0f;
    float g = (float)color.g / 255.0f;
    float b = (float)color.b / 255.0f;


    float texc_y_0 = 0.0f;
    float texc_y_1 = 1.0f;

    float texc_x_0 = 0.0f;
    float texc_x_1 = 1.0f;
    
    if(options & LEAF_TEXTURE_FLIP_VERTICAL) {
        texc_y_0 = 1.0f;
        texc_y_1 = 0.0f;
    }

    if(options & LEAF_TEXTURE_FLIP_HORIZONTAL) {
        texc_x_0 = 1.0f;
        texc_x_1 = 0.0f;
    }


    float vertices[] = {
        x,   y+h, r, g, b,  texc_x_0, texc_y_1,
        x,   y,   r, g, b,  texc_x_0, texc_y_0,
        x+w, y,   r, g, b,  texc_x_1, texc_y_0,

        x,   y+h, r, g, b,  texc_x_0, texc_y_1,
        x+w, y,   r, g, b,  texc_x_1, texc_y_0,
        x+w, y+h, r, g, b,  texc_x_1, texc_y_1
    };

    glUseProgram(g_leaf_ctx->renderer_tex_shader); 
    glBindVertexArray(g_leaf_ctx->renderer_tex_vao);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_ARRAY_BUFFER, g_leaf_ctx->renderer_tex_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void leaf_draw_circle
(
    float pos_x,
    float pos_y,
    float radius,
    int num_triangles,
    struct color_t color
){
    float center_x = 0;
    float center_y = 0;

    leaf_normalize_coords(pos_x, pos_y, &center_x, &center_y);

    float r = (float)color.r / 255.0f;
    float g = (float)color.g / 255.0f;
    float b = (float)color.b / 255.0f;

    float angle = 0;
    for(int i = 0; i < num_triangles; ) {
 
        angle = 2.0 * M_PI * i / num_triangles;

        float now_x = 0;
        float now_y = 0;
        float next_x = 0;
        float next_y = 0;
        
        angle = 2.0 * M_PI * i / num_triangles;
        leaf_normalize_coords(
                pos_x + radius * cos(angle),
                pos_y + radius * sin(angle),
                &now_x,
                &now_y);

        i++;

        angle = 2.0 * M_PI * i / num_triangles;
        leaf_normalize_coords(
                pos_x + radius * cos(angle),
                pos_y + radius * sin(angle),
                &next_x,
                &next_y);

        float vertices[] = {
            center_x, center_y, r, g, b,
            now_x, now_y,       r, g, b,
            next_x, next_y,     r, g, b
        };
        leaf_render_vertices(g_leaf_ctx, vertices, sizeof(vertices));
    }
}

void leaf_draw_rect_fade
(
    float pos_x,
    float pos_y,
    float width,
    float height,
    struct color_t color_A,
    struct color_t color_B,
    int fade_dir
){
    pos_y = g_leaf_ctx->win_height - pos_y;
    float x = (pos_x / (float)g_leaf_ctx->win_width) * 2.0f - 1.0f;
    float y = (pos_y / (float)g_leaf_ctx->win_height) * 2.0f - 1.0f;

    float w = width / ((float)g_leaf_ctx->win_width / 2.0f);
    float h = height / ((float)g_leaf_ctx->win_height / 2.0f);

    float rA = (float)color_A.r / 255.0f;
    float gA = (float)color_A.g / 255.0f;
    float bA = (float)color_A.b / 255.0f;

    float rB = (float)color_B.r / 255.0f;
    float gB = (float)color_B.g / 255.0f;
    float bB = (float)color_B.b / 255.0f;

    if(fade_dir == LEAF_RECT_FADE_HORIZONTAL) {
        float vertices[] = {    
            x,   y-h, rB, gB, bB,
            x,   y,   rA, gA, bA,
            x+w, y,   rA, gA, bA,

            x,   y-h, rB, gB, bB,
            x+w, y,   rA, gA, bA,
            x+w, y-h, rB, gB, bB
        };       
    
        leaf_render_vertices(g_leaf_ctx, vertices, sizeof(vertices));
    }
    else 
    if(fade_dir == LEAF_RECT_FADE_VERTICAL) {
        float vertices[] = {    
            x,   y-h, rA, gA, bA,
            x,   y,   rA, gA, bA,
            x+w, y,   rB, gB, bB,

            x,   y-h, rA, gA, bA,
            x+w, y,   rB, gB, bB,
            x+w, y-h, rB, gB, bB
        };
        leaf_render_vertices(g_leaf_ctx, vertices, sizeof(vertices));
    }
}


