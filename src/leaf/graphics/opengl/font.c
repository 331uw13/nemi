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

#ifdef GRAPHICS_OPENGL

#include <stdio.h>
#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../thirdparty/stb_image_write.h"

#include "../../leaf.h"
#include "../../font.h"
#include "shaders.h"


static const char FONT_VERTEX_SHADER_SRC[] = {
    "#version 330 core\n"    
    "layout (location = 0) in vec2 in_pos;\n"
    "layout (location = 1) in vec2 in_tex_coords;\n"
    "layout (location = 2) in float in_atlas_offset;\n"
    "layout (location = 3) in vec3  in_char_color;\n"
    "layout (location = 4) in float in_char_origin_y;\n"
    "layout (location = 5) in float in_char_italic;\n"
    "layout (location = 6) in float in_char_bold;\n"
    "uniform float u_scale;\n"
   
    "out vec2 tex_coords;\n" 
    "out vec3 color;\n"
    "out float atlas_offset;\n"
    "out float glyph_width;\n"
    "out float char_bold;\n"
    "void main() {\n"
        "tex_coords   = in_tex_coords;\n"
        "atlas_offset = in_atlas_offset;\n"
        "color        = in_char_color;\n"
        "char_bold = in_char_bold;\n"
        
        "vec2 p = in_pos;\n"
        "p.x += (p.y - in_char_origin_y) * (in_char_italic * 0.5);\n"
        "gl_Position = vec4(p.x, p.y, 0.0, 1.0);\n"
    "}\n"
};

static const char FONT_FRAGMENT_SHADER_SRC[] = {
    "#version 330 core\n"    
    "out vec4 out_color;\n"  
    "in vec2 tex_coords;\n"  
    "in vec3 color;\n"
    "in float atlas_offset;\n"
    "in float glyph_width;\n"
    "in float char_bold;\n"

    "uniform float u_tex_w;\n"
    "uniform float u_tex_h;\n"
    "uniform float u_chr_w;\n"
    "uniform float u_chr_h;\n"
    "uniform sampler2D tex;\n"

    "void main() {\n"
        "float char_w = u_chr_w / u_tex_w;\n"
        "float char_h = u_chr_h / u_tex_h + 0.1;\n" // Small offset to zoom character height a bit out
                                                  // to make room for characters who go a bit outside
                                                  // for example: ;j and g
        "float tex_x = (atlas_offset + tex_coords.x) * char_w;\n"
        "float tex_y =                 tex_coords.y  * char_h;\n"
        "vec2 texc = vec2(tex_x, tex_y);\n"
        "float c = texture(tex, texc).r;\n"
        // This fixes the issue with font getting very small
        // it becomes unreadable very fast.
        // It looks better when sampling nearby pixels and mixing them.
        "float px = 1.0f / u_tex_w;\n"
        "float py = 1.0f / u_tex_h;\n"
        "float s01 = texture(tex, texc + vec2(px, 0)).r;\n"
        "float s02 = texture(tex, texc + vec2(-px, 0)).r;\n"
        "c = mix(c, mix(s01, s02, 0.5f), 0.5f);\n"

        // The bold setting works kind of the same way.
        "float b01 = texture(tex, texc + vec2(px, 0)).r   * 0.9;"
        "float b03 = texture(tex, texc + vec2(px*2, 0)).r * 0.4;"
        "float b02 = texture(tex, texc + vec2(-px, 0)).r  * 0.9;"
        "float b04 = texture(tex, texc + vec2(-px*2, 0)).r * 0.4;"
        "float b10 = texture(tex, texc + vec2(0, py)).r    * 0.9;"
        "float b20 = texture(tex, texc + vec2(0, -py)).r   * 0.9;"
        "float bs = 0.8;"
        "c += (b01*bs + b02*bs + b10*bs + b20*bs + b03*bs + b04*bs) * char_bold;"
        
        "float a = smoothstep(0.0f, 0.8f, c);\n"
        "out_color = vec4(color, a);\n"
    "}\n"
};

void leaf_font_render(LeafFont* font) {

    glBindVertexArray(font->vao);
    glUseProgram(font->shader);

    shader_uniform1f(font->shader, "u_tex_w", font->texture_width);
    shader_uniform1f(font->shader, "u_tex_h", font->texture_height);
    shader_uniform1f(font->shader, "u_chr_w", font->real_char_width);
    shader_uniform1f(font->shader, "u_chr_h", font->real_char_height);
    shader_uniform1f(font->shader, "u_scale", font->scale);
    //shader_uniform1f(font->shader, "u_bold", font->bold);
    /*
    shader_uniform1f(font->shader, "u_chr_w", 
            (float)font->real_char_width / (float)lfctx->win_width);
    */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glDrawArrays(GL_TRIANGLES, 0, font->vbo_num_vertices);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    font->vbo_data_offset = 0;
    font->vbo_num_vertices = 0;
}

bool leaf_load_font(LeafFont* font, const char* filepath) {
    int res = 0;

    FT_Library ft;

    if(FT_Init_FreeType(&ft)) {
        fprintf(stderr, "%s: Failed to initialize freetype library.\n", __func__);
        goto error;
    }


    FT_Face face;

    if(FT_New_Face(ft, filepath, 0, &face)) {
        fprintf(stderr, "%s: Failed to load font from '%s'.\n", __func__, filepath);
        FT_Done_FreeType(ft);
        goto error;
    }

    FT_Set_Pixel_Sizes(face, 0, 64);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font->real_char_width = 0;
    font->real_char_height = 0;
    font->shader = 0;
    font->texture = 0;
    font->texture_width = 0;
    font->texture_height = 0;
    font->center_char_to_cell = false;
    font->vbo_data_offset = 0;

    font->shader = create_shader_program(
            FONT_VERTEX_SHADER_SRC, 
            FONT_FRAGMENT_SHADER_SRC);
    
    if(!font->shader) {
        fprintf(stderr, "--- Font Vertex Shader ---\n%s\n", 
                FONT_VERTEX_SHADER_SRC);
        
        fprintf(stderr, "\n--- Font Fragment Shader ---\n%s\n", 
                FONT_FRAGMENT_SHADER_SRC);
        goto error_and_done;
    }


    // TODO: Some fonts maybe dont follow maybe same offsets ? not sure yet...
    const uint32_t first_ascii_char = 0x20;
    const uint32_t last_ascii_char = 0x7F;

    //FT_Bitmap bitmaps [FONT_NUM_CHARS];
    //size_t bitmaps_idx = 0;

    int max_bearing_top = INT_MIN;

    // Load characters first
    // and get the font character max width and height.
    for(uint32_t c = first_ascii_char; c < last_ascii_char; c++) {

        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "%s: FT_Load_Char failed '%c'\n font_path: '%s'\n",
                    __func__, c, filepath);
            continue;
        }

        uint32_t bitmap_width = face->glyph->bitmap.width;
        uint32_t bitmap_height = face->glyph->bitmap.rows;

        // Keep track of max char width and height.
        if(bitmap_width > font->real_char_width) {
            font->real_char_width = bitmap_width;
        }
        if(bitmap_height > font->real_char_height) {
            font->real_char_height = bitmap_height;
        }

        // Keep track of max bearing top
        // for calculating the texture atlas height later.
        if(max_bearing_top < face->glyph->bitmap_top) {
            max_bearing_top = face->glyph->bitmap_top;
        }

        LeafFontGlyph* glyph = &font->glyphs[c - 0x20];
        *glyph = (LeafFontGlyph) {
            .width = bitmap_width,
            .height = bitmap_height,
            .bearing_x = face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top,
            .atlas_x = 0
        };

        //bitmaps[bitmaps_idx++] = face->glyph->bitmap;
    }

    // We are going to create very wide texture
    // where all characters are in the same row

    font->texture_width  = FONT_NUM_CHARS * font->real_char_width;
    font->texture_height = font->real_char_height + max_bearing_top * 2;


    size_t num_pixels = font->texture_width * font->texture_height;
    uint8_t* pixels = calloc(num_pixels, sizeof *pixels);

   
    uint32_t char_index = 0;

    for(uint32_t c = first_ascii_char; c < last_ascii_char; c++) { 
        //FT_Bitmap* bitmap = &bitmaps[c_index];


        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "%s: FT_Load_Char failed '%c'\n font_path: '%s'\n",
                    __func__, c, filepath);
            continue;
        }

        FT_Bitmap* bitmap = &face->glyph->bitmap;

        for(uint32_t row = 0; row < bitmap->rows; row++) {
            for(uint32_t col = 0; col < bitmap->width; col++) {
                
                int y = row;
                int x = col + char_index * font->real_char_width;

                // Offset the character to correct Y position.
                // For example: ", - * +". are at different y levels
                y += (font->real_char_height - bitmap->rows);
                y += (bitmap->rows - face->glyph->bitmap_top);
                
                x += (font->real_char_width - bitmap->width) / 3;

                int    byte = bitmap->buffer[row * bitmap->width + col];
                size_t idx  = y * font->texture_width + x;
                pixels[idx] = byte;
            }
        }
        
        LeafFontGlyph* glyph = &font->glyphs[char_index++];
        glyph->atlas_x = char_index-1;
    }

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexImage2D(
            GL_TEXTURE_2D,
            0, GL_RED,
            font->texture_width,
            font->texture_height,
            0, GL_RED, GL_UNSIGNED_BYTE,
            pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(pixels);
  
    glGenVertexArrays(1, &font->vao);
    glBindVertexArray(font->vao);


    // vertex_x,
    // vertex_y,
    // texture_x,
    // texture_y,
    // atlas_offset,
    // char red, 
    // char green,
    // char blue,
    // char origin y
    // char italic
    const int stride_num_floats = 2 + 2 + 1 + 3 + 1 + 1 + 1;

    font->vbo_memsize = (100 * 100) * (sizeof(float) * (stride_num_floats * 6));
    glGenBuffers(1, &font->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferData(GL_ARRAY_BUFFER, font->vbo_memsize, NULL, GL_DYNAMIC_DRAW);
   
    
    const size_t stride_size = stride_num_floats * sizeof(float);

    // Positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
            stride_size, 0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
 
    // Texture atlas X offset
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(2);

    // Character color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(3);

    // Character origin y
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(4);

    // Character italic
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(9*sizeof(float)));
    glEnableVertexAttribArray(5);
    
    // Character bold
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(10*sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
    

    font->italic = 0.0f;
    font->bold   = 0.0f;
    font->char_width = 0;
    font->char_height = 0;
    leaf_set_font_scale(font, 4.0f);
    leaf_set_font_color(font, (RGBColor){ 255, 255, 255 });
  
    // Set default values.
    leaf_set_font_spacing(font, 1.0f);
    leaf_set_font_space_width(font, 8);
    leaf_set_font_tab_width(font, 8*4);


    printf("%s: LeafFont texture: %ix%i\n", __FILE__,
            font->texture_width,
            font->texture_height);

    font->loaded = true;
    res = 1;

error_and_done:

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


error:
    return res;

}


void leaf_unload_font(LeafFont* font) {
    if(!font) {
        return;
    }

    /*
    for(int i = 0; i < FONT_NUM_CHARS; i++) {
        LeafFontGlyph* glyph = &font->glyphs[i];
    }
    */

    glDeleteProgram(font->shader);
    glDeleteBuffers(1, &font->vbo);
    glDeleteVertexArrays(1, &font->vao);
    glDeleteTextures(1, &font->texture);
}

/*
void leaf_set_font_scale(LeafFont* font, float scale) {
    font->scale = scale;
    font->char_width = (font->real_char_width * scale) / 2;
    font->char_height = (font->real_char_height * scale) / 2;
   
    leaf_set_font_space_width(font, font->real_space_width);
    leaf_set_font_tab_width(font, font->real_tab_width);
}

void leaf_set_font_color(LeafFont* font, RGBColor color) {
    font->char_color_r = (float)color.r / 255.0f;
    font->char_color_g = (float)color.g / 255.0f;
    font->char_color_b = (float)color.b / 255.0f;
}

void leaf_set_font_space_width(LeafFont* font, float space_width) {
    font->real_space_width = space_width;
    font->space_width = space_width * font->scale;
}

void leaf_set_font_tab_width(LeafFont* font, float tab_width) {
    font->real_tab_width = tab_width;
    font->tab_width = tab_width * font->scale;
}

void leaf_set_font_spacing(LeafFont* font, float spacing) {
    font->spacing = spacing * font->scale;
}

void leaf_measure_text
(
    LeafFont* font, 
    int* width_out,
    int* height_out,
    const char* text,
    ssize_t text_length
){

    if(text_length < 0) {
        text_length = strlen(text);
    }

    *width_out = 0;
    *height_out = font->char_height / 2; // TODO: Take in count '\n'


    for(ssize_t i = 0; i < text_length; i++) {
        char ch = text[i];
        if((ch < 0x20) || (ch > 0x7E)) {
            continue; // Not ASCII character.
        }
        
        if(ch == 0x20) {
            *width_out += font->space_width;
            continue;
        }
        else
        if(ch == 0x09) {
            *width_out += font->tab_width;
            continue;
        }

        LeafFontGlyph* glyph = &font->glyphs[ch - 0x20];

        *width_out += glyph->width * font->scale / 2;
        *width_out += font->spacing;
    }

}
*/


#endif // GRAPHICS_OPENGL
