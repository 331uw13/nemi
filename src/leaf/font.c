
#include <stdio.h>
#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"


#include "font.h"
#include "shaders.h"
#include "leaf.h"



static const char FONT_VERTEX_SHADER_SRC[] = {
    "#version 330 core\n"    
    "layout (location = 0) in vec2 in_pos;\n"
    "layout (location = 1) in vec2 in_tex_coords;\n"
    "layout (location = 2) in float in_atlas_offset;\n"
    "layout (location = 3) in vec3  in_char_color;\n"
    //"uniform vec3 font_color;\n"
    //"uniform float u_italic;\n"
    //"uniform float u_char_y;\n"
    //"uniform float u_char_x;\n"
    "uniform float u_scale;\n"
    
    "out vec2 tex_coords;\n" 
    "out vec3 color;\n"
    "out float atlas_offset;\n"
    "out float glyph_width;\n"
    "\n"
    "void main() {\n"
        "tex_coords   = in_tex_coords;\n"
        "atlas_offset = in_atlas_offset;\n"
        "color        = in_char_color;\n"
        
        "vec2 p = in_pos;\n"
        "gl_Position = vec4(p.x, p.y, 0.0, 1.0);\n"
    //"   color = vec3(1, 1, 1);\n"
    //"   p.x += (p.y - u_char_y) * (u_italic * 0.5);\n"
    //"   p.x += u_char_x;\n"
    //"   p.y += u_char_y;\n"
    //"   tex_coords = texture_coords;\n"
    //"   gl_Position = vec4(p.x, p.y, 0.0, 1.0);\n"
    //"   \n"
    "}\n"
};

static const char FONT_FRAGMENT_SHADER_SRC[] = {
    "#version 330 core\n"    
    "out vec4 out_color;\n"  
    "in vec2 tex_coords;\n"  
    "in vec3 color;\n"
    "in float atlas_offset;\n"
    "in float glyph_width;\n"
    
    "uniform float u_tex_w;\n"
    "uniform float u_tex_h;\n"
    "uniform float u_chr_w;\n"
    "uniform float u_chr_h;\n"
    "uniform sampler2D tex;\n"
  
    "void main() {\n"
        "float char_w = u_chr_w / u_tex_w;"
        "float char_h = u_chr_h / u_tex_h;"
        "float tex_x = (atlas_offset + tex_coords.x) * char_w;"
        "float tex_y =                 tex_coords.y  * char_h;"
        "vec2 texc = vec2(tex_x, tex_y * 1.2);"
        "float c = texture(tex, texc).r;"
        
        "float a = smoothstep(0.1f, 1.0f, c);"
        "out_color = vec4(color, a);"

    //"    vec4 t = texture(tex, tex_coords);\n"
    //"    vec3 c = color * t.r;\n"
    //"    float a = smoothstep(0.1f, 1.0f, t.r);"
    //"    out_color = vec4(c, a);\n"
    "}\n"
};

void leaf_font_render(struct leaf_ctx_t* lfctx, struct font_t* font) {

    glBindVertexArray(font->vao);
    glUseProgram(font->shader);

    shader_uniform1f(font->shader, "u_tex_w", font->texture_width);
    shader_uniform1f(font->shader, "u_tex_h", font->texture_height);
    shader_uniform1f(font->shader, "u_chr_w", font->max_bitmap_width);
    shader_uniform1f(font->shader, "u_chr_h", font->max_bitmap_height);
    shader_uniform1f(font->shader, "u_scale", font->scale);
    /*
    shader_uniform1f(font->shader, "u_chr_w", 
            (float)font->max_bitmap_width / (float)lfctx->win_width);
    */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glDrawArrays(GL_TRIANGLES, 0, font->vbo_num_vertices);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //printf("%s: vbo_data_offset = %li\n", __func__, font->vbo_data_offset);
    font->vbo_data_offset = 0;
    font->vbo_num_vertices = 0;
}

bool leaf_load_font(struct leaf_ctx_t* lfctx, struct font_t* font, const char* filepath) {
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

    FT_Set_Pixel_Sizes(face, 0, 32);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font->max_bitmap_width = 0;
    font->max_bitmap_height = 0;
    font->shader = 0;
    font->texture = 0;
    font->texture_width = 0;
    font->texture_height = 0;
    font->center_char_to_cell = false;
    font->italic = 0.0f;
    font->vbo_data_offset = 0;

    font->shader = create_shader_program(
            FONT_VERTEX_SHADER_SRC, 
            FONT_FRAGMENT_SHADER_SRC);
    
    if(!font->shader) {
        goto error_and_done;
    }


    const uint32_t first_ascii_char = 0x20;
    const uint32_t last_ascii_char = 0x7F;

    //FT_Bitmap bitmaps [FONT_NUM_CHARS];
    //size_t bitmaps_idx = 0;

    // Load characters first
    // and get the font character max width and height.
    for(uint32_t c = first_ascii_char; c < last_ascii_char; c++) {

        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "%s: FT_Load_Char failed '%c'\n font_path: '%s'\n",
                    __func__, c, filepath);
            continue;
        }

        int bitmap_width = face->glyph->bitmap.width;
        int bitmap_height = face->glyph->bitmap.rows;

        if(bitmap_width > font->max_bitmap_width) {
            font->max_bitmap_width = bitmap_width;
        }
        if(bitmap_height > font->max_bitmap_height) {
            font->max_bitmap_height = bitmap_height;
        }

        struct glyph_t* glyph = &font->glyphs[c - 0x20];
        *glyph = (struct glyph_t) {
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

    font->texture_width  = FONT_NUM_CHARS * font->max_bitmap_width;
    font->texture_height  = font->max_bitmap_height;

    // Add safe amount of extra space for characters
    // in y axis because they have to be offset later
    font->texture_height += 60;

    size_t num_pixels = font->texture_width * font->texture_height;
    uint8_t* pixels = calloc(num_pixels*4 /*rgba*/, sizeof *pixels);

    printf("Font texture: %ix%i\n",
            font->texture_width,
            font->texture_height);
    
    uint32_t c_index = 0;


    int tex_actual_width = 0;


    for(uint32_t c = first_ascii_char; c < last_ascii_char; c++) { 
        //FT_Bitmap* bitmap = &bitmaps[c_index];


        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "%s: FT_Load_Char failed '%c'\n font_path: '%s'\n",
                    __func__, c, filepath);
            continue;
        }

        FT_Bitmap* bitmap = &face->glyph->bitmap;

        for(int row = 0; row < bitmap->rows; row++) {
            for(int col = 0; col < bitmap->width; col++) {
                
                int y = row;
                int x = col + tex_actual_width;

                // Offset the character to correct Y position.
                // For example: ", - * +". are at different y levels
                y += (font->max_bitmap_height - bitmap->rows);
                y += (bitmap->rows - face->glyph->bitmap_top);
                
                x += (font->max_bitmap_width - bitmap->width) / 3;

                int byte = bitmap->buffer[row * bitmap->width + col];
                size_t idx = y * font->texture_width + x;
                pixels[idx * 4 + 0] = byte;
                pixels[idx * 4 + 1] = byte;
                pixels[idx * 4 + 2] = byte;
                pixels[idx * 4 + 3] = 0xFF;
            }
        }
        
        struct glyph_t* glyph = &font->glyphs[c_index++];
        glyph->atlas_x = c_index-1;
        
        tex_actual_width += font->max_bitmap_width;
    }

        
    printf("TODO: Add option for font_center_char_to_cell\n");

    // Writing to disk temporarily to just make
    // sure everything else is working first.

    stbi_write_png("font.png", 
            tex_actual_width,
            font->texture_height,
            4,
            pixels,
            font->texture_width * 4);

    // font->texture_width = tex_actual_width;

    font->texture = leaf_load_texture("font.png", 
            &font->texture_width,
            &font->texture_height);
    

    /*
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
    */

    free(pixels);
        /*
        uint32_t tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D,
                0, GL_RED,
                bitmap_width,
                bitmap_height,
                0, GL_RED, GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        struct glyph_t* glyph = &font->glyphs[ c - 0x20 ];
        *glyph = (struct glyph_t) {
            .texture = tex,
            .width = bitmap_width,
            .height = bitmap_height,
            .bearing_x = face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top
        };

        float cw = bitmap_width / (float)lfctx->win_width;
        float ch = bitmap_height / (float)lfctx->win_height;

        float glyph_vertices[] = {
            0,    0-ch, 0.0f, 1.0f,
            0,    0,    0.0f, 0.0f,
            0+cw, 0,    1.0f, 0.0f,

            0,    0-ch, 0.0f, 1.0f,
            0+cw, 0,    1.0f, 0.0f,
            0+cw, 0-ch, 1.0f, 1.0f
        };

        */

 
    glGenVertexArrays(1, &font->vao);
    glBindVertexArray(font->vao);


    const int stride_num_floats = 2 + 2 + 1 + 3;

    glGenBuffers(1, &font->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
            (sizeof(float) * (stride_num_floats * 6)) * (600 * 600),
            NULL, GL_DYNAMIC_DRAW);
    
    // x, y, tex_x, tex_y, atlas_x,
    const size_t stride_size = stride_num_floats * sizeof(float);

    // Positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, 0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
 
    // Texture atlas coordinate X offset
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(2);

    // Character color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
            stride_size, (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(3);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
    

    font->char_width = 0;
    font->char_height = 0;
    leaf_set_font_scale(font, 4.0f);
    leaf_set_font_color(font, 1.0f, 1.0f, 1.0f);
  
    // Set default values.
    leaf_set_font_spacing(font, 1.0f);
    leaf_set_font_space_width(font, 8);
    leaf_set_font_tab_width(font, 8*4);

error_and_done:

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    res = 1;

error:
    return res;

}


void leaf_unload_font(struct font_t* font) {
    if(!font) {
        return;
    }

    for(int i = 0; i < FONT_NUM_CHARS; i++) {
        struct glyph_t* glyph = &font->glyphs[i];

        /*if(glyph->texture > 0) {
            glDeleteTextures(1, &glyph->texture);
            glyph->texture = 0;
        
        }*/
         
        //glDeleteBuffers(1, &glyph->vbo);
        //glDeleteVertexArrays(1, &glyph->vao);
    }

    glDeleteProgram(font->shader);
    glDeleteBuffers(1, &font->vbo);
    glDeleteVertexArrays(1, &font->vao);
    glDeleteTextures(1, &font->texture);
}

void leaf_set_font_scale(struct font_t* font, float scale) {
    font->scale = scale;
    font->char_width = (font->max_bitmap_width * scale) / 2;
    font->char_height = (font->max_bitmap_height * scale) / 2;
   
    leaf_set_font_space_width(font, font->real_space_width);
    leaf_set_font_tab_width(font, font->real_tab_width);
}

void leaf_set_font_color(struct font_t* font, float r, float g, float b) {
    font->char_color_r = r;
    font->char_color_g = g;
    font->char_color_b = b;
    //glUseProgram(font->shader);
    //glUniform3f(font->shader_color_uniloc, r, g, b);
}

void leaf_set_font_space_width(struct font_t* font, float space_width) {
    font->real_space_width = space_width;
    font->space_width = space_width * font->scale;
}

void leaf_set_font_tab_width(struct font_t* font, float tab_width) {
    font->real_tab_width = tab_width;
    font->tab_width = tab_width * font->scale;
}

void leaf_set_font_spacing(struct font_t* font, float spacing) {
    font->spacing = spacing * font->scale;
}

void leaf_measure_text
(
    struct font_t* font, 
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
        
        if(ch == 0x20/*space*/) {
            *width_out += font->space_width;
            continue;
        }
        else
        if(ch == 0x09/*horizontal tab*/) {
            *width_out += font->tab_width;
            continue;
        }

        struct glyph_t* glyph = &font->glyphs[ch - 0x20];

        *width_out += glyph->width * font->scale / 2;
        *width_out += font->spacing;
    }

}



