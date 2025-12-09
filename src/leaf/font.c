
#include <stdio.h>
#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "font.h"
#include "shaders.h"


static const char FONT_VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"    
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 texture_coords;\n"
    "uniform vec3 font_color;\n"
    "out vec2 tex_coords;\n" 
    "out vec3 color;\n"
    "\n"
    "void main() {\n"
    "    color = font_color;\n"
    "    tex_coords = texture_coords;\n"
    "    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n",
};

static const char FONT_FRAGMENT_SHADER_SRC[] = {
    "#version 460 core\n"    
    "out vec4 out_color;\n"  
    "in vec2 tex_coords;\n"  
    "in vec3 color;\n"
    "\n"
    "uniform sampler2D tex;\n"
    "\n"
    "void main() {\n"
    "    vec4 t = texture(tex, tex_coords);\n"
    "    vec3 c = color * t.r;\n"
    "    float a = smoothstep(0.1f, 1.0f, t.r);"
    "    out_color = vec4(c, a);\n"
    "}\n",
};


bool leaf_load_font(struct font_t* font, const char* filepath) {
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

    // ok good, now start settings stuff up.


    FT_Set_Pixel_Sizes(face, 0, 32);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font->max_bitmap_width = 0;
    font->max_bitmap_height = 0;
    font->shader = 0;
    font->vbo = 0;
    font->vao = 0;
    font->shader_color_uniloc = -1;
    font->center_char_to_cell = false;

    font->shader = create_shader_program(FONT_VERTEX_SHADER_SRC, FONT_FRAGMENT_SHADER_SRC);
    if(!font->shader) {
        goto error_and_done;
    }

    glGenVertexArrays(1, &font->vao);
    glBindVertexArray(font->vao);

    glGenBuffers(1, &font->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    const size_t stride_size = 4 * sizeof(float);

    // Positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, 0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride_size, (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    font->shader_color_uniloc =
        glGetUniformLocation(font->shader, "font_color");

    if(font->shader_color_uniloc < 0) {
        fprintf(stderr, "\033[33m'load_font'(warning): uniform location not found.\033[0m\n");
    }

    for(uint32_t c = 0x20; c < 0x7F; c++) {

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

        //printf("%s: %c | %i\n",__func__, c, tex);
        font->glyphs[c - 0x20] = (struct glyph_t) {
            .texture = tex,
            .width = bitmap_width,
            .height = bitmap_height,
            .bearing_x = face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top
            //.test = face->glyph->metrics.width
        };
    }

   
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
        if(font->glyphs[i].texture > 0) {
            glDeleteTextures(1, &font->glyphs[i].texture);
            font->glyphs[i].texture = 0;
        }
    }

    glDeleteProgram(font->shader);
    glDeleteBuffers(1, &font->vbo);
    glDeleteVertexArrays(1, &font->vao);
}

void leaf_set_font_scale(struct font_t* font, float scale) {
    font->scale = scale;
    font->char_width = (font->max_bitmap_width * scale) / 2;
    font->char_height = (font->max_bitmap_height * scale) / 2;
   
    leaf_set_font_space_width(font, font->real_space_width);
    leaf_set_font_tab_width(font, font->real_tab_width);
}

void leaf_set_font_color(struct font_t* font, float r, float g, float b) {
    glUseProgram(font->shader);
    glUniform3f(font->shader_color_uniloc, r, g, b);
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



