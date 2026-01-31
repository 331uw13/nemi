#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include "leaf.h"
#include "shaders.h"

static const char RENDERER_VERTEX_SHADER_SRC[] = {
    "#version 330 core\n"
    "layout (location = 0) in vec2 vertex_pos;\n"
    "layout (location = 1) in vec3 vertex_color;\n"
    "out vec3 v_color;\n"
    "\n"
    "void main() {\n"
    "    v_color = vertex_color;\n"
    "    gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n",
};

static const char RENDERER_FRAGMENT_SHADER_SRC[] = {
    "#version 330 core\n"    
    "out vec4 out_color;\n"  
    "in vec3 v_color;\n"
    "\n"
    "void main() {\n"
    "    out_color = vec4(v_color, 1.0);\n"
    "}\n",
};


static const char RENDERER_TEX_VERTEX_SHADER_SRC[] = {
    "#version 330 core\n"
    "layout (location = 0) in vec2 vertex_pos;\n"
    "layout (location = 1) in vec3 vertex_color;\n"
    "layout (location = 2) in vec2 tex_coords;\n"
    "out vec3 v_color;\n"
    "out vec2 v_tex_coords;\n"
    "\n"
    "void main() {\n"
    "    v_color = vertex_color;\n"
    "    v_tex_coords = tex_coords;\n"
    "    gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0.0, 1.0);\n"
    "    \n"
    "}\n"
};

static const char RENDERER_TEX_FRAGMENT_SHADER_SRC[] = {
    "#version 330 core\n"    
    "out vec4 out_color;\n"  
    "in vec3 v_color;\n"
    "in vec2 v_tex_coords;\n"
    "uniform sampler2D tex;\n"
    "\n"
    "void main() {\n"
    "    vec4 t = texture(tex, vec2(v_tex_coords.x, v_tex_coords.y));"
    "    out_color = vec4(t.rgb * v_color, t.a);\n"
    "}\n"
};


struct leaf_ctx_t* leaf_open(const char* title, int width, int height, int flags) {
    struct leaf_ctx_t* ctx = malloc(sizeof *ctx);
    if(!ctx) {
        goto out;
    }

    ctx->glfw_win = NULL;
    ctx->win_width = width;
    ctx->win_height = height;

    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialzie GLFW\n");
        goto out;
    }


    if(flags & LEAF_NORESIZE) {
        glfwWindowHint(GLFW_RESIZABLE, false);
    }

    ctx->glfw_win = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!ctx->glfw_win) {
        fprintf(stderr, "Failed to create window\n");
        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }

    if(flags & LEAF_NORESIZE) {
        glfwSetWindowSizeLimits(ctx->glfw_win, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }

    glfwMakeContextCurrent(ctx->glfw_win);
    
    GLenum glew_err = glewInit();
    if(glew_err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW | %s\n", 
                glewGetErrorString(glew_err));
        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }

    leaf_set_drawing_context(ctx);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    ctx->renderer_vao = 0;
    ctx->renderer_vbo = 0;
    ctx->renderer_shader = 0;
    ctx->renderer_num_vertex_positions = 0;
    ctx->renderer_vbo_memsize = 0;
    ctx->renderer_vbo_size = 0;

out:
    return ctx;
}


void leaf_quit(struct leaf_ctx_t* ctx) {
    if(!ctx) {
        return;
    }

    if(ctx->glfw_win) {
        glfwDestroyWindow(ctx->glfw_win);
    }

    glfwTerminate();
    free(ctx);
}

void leaf_init_renderer(struct leaf_ctx_t* ctx, size_t vbo_memsize) {    
    glGenVertexArrays(1, &ctx->renderer_vao);
    glBindVertexArray(ctx->renderer_vao);

    glGenBuffers(1, &ctx->renderer_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_memsize, NULL, GL_STATIC_DRAW);

    ctx->renderer_vbo_memsize = vbo_memsize;
    ctx->renderer_vbo_data_offset = 0;
    ctx->renderer_vbo_num_vertices = 0;
    size_t stride_size = (2 + 3) * sizeof(float);


    // Positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, (void*)(0));
    glEnableVertexAttribArray(0);

    // Colors.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride_size, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);




    // For textures
 
    glGenVertexArrays(1, &ctx->renderer_tex_vao);
    glBindVertexArray(ctx->renderer_tex_vao);

    glGenBuffers(1, &ctx->renderer_tex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_tex_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_memsize, NULL, GL_STATIC_DRAW);

    //ctx->renderer_vbo_memsize = vbo_memsize;


    stride_size = (2 + 3 + 2) * sizeof(float);
    // Positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, NULL);
    glEnableVertexAttribArray(0);

    // Colors.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride_size,
            (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinates.
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride_size,
            (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

   

    // Compile rendering shaders.
    ctx->renderer_shader = create_shader_program
        (RENDERER_VERTEX_SHADER_SRC, RENDERER_FRAGMENT_SHADER_SRC);

    // Compile rendering shaders.
    ctx->renderer_tex_shader = create_shader_program
        (RENDERER_TEX_VERTEX_SHADER_SRC, RENDERER_TEX_FRAGMENT_SHADER_SRC);
}

void leaf_free_renderer(struct leaf_ctx_t* ctx) {
    if(ctx->renderer_vao > 0) {
        glDeleteVertexArrays(1, &ctx->renderer_vao);
        ctx->renderer_vao = 0;
    }
    if(ctx->renderer_vbo > 0) {
        glDeleteBuffers(1, &ctx->renderer_vbo);
        ctx->renderer_vbo = 0;
    }
    if(ctx->renderer_shader > 0) {
        glDeleteProgram(ctx->renderer_shader);
        ctx->renderer_shader = 0;
    }
}

void leaf_render_vertices(struct leaf_ctx_t* ctx, float* vertices, size_t vertices_memsize) {   
    bool divisible = !(vertices_memsize % 5); // 2(x, y) + 3(r, g, b) = 5
    if(!divisible) {
        fprintf(stderr, "(%s) %s(): Vertex data format is incorrect.\n",
                __FILE__, __func__);
        return;
    }

    /*
    if(vertices_memsize >= ctx->renderer_vbo_memsize) {
        fprintf(stderr, "(%s) %s(): Renderer VBO doesnt have enough memory allocated.\n",
                __FILE__, __func__);
        return;
    }
    */

    /*
    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ctx->renderer_vbo_data_offset, vertices_memsize, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ctx->renderer_vbo_data_offset += vertices_memsize;
    ctx->renderer_vbo_num_vertices += 6;//((vertices_memsize / sizeof(float)) / 5);
    */
    
    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_memsize, vertices);
    
    const int num_vertex_positions = ((vertices_memsize / sizeof(float)) / 5) * 2;
   
    glBindVertexArray(ctx->renderer_vao);
   
    glUseProgram(ctx->renderer_shader);
    glDrawArrays(GL_TRIANGLES, 0, num_vertex_positions);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void leaf_renderer_flush(struct leaf_ctx_t* ctx) {

    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_vbo);
    glBindVertexArray(ctx->renderer_vao);
    glUseProgram(ctx->renderer_shader);

    glDrawArrays(GL_TRIANGLES, 0, ctx->renderer_vbo_num_vertices);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ctx->renderer_vbo_data_offset = 0;
    ctx->renderer_vbo_num_vertices = 0;
}

uint32_t leaf_load_texture(const char* path, int* width, int* height) {
    uint32_t tex = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);
    uint8_t* image = stbi_load(path, width, height, &channels, 0);

    printf("%s(): %s: %i, %i\n", __func__, path, *width, *height);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int ch = GL_RGB;
    if(channels == 4) {
        ch = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D,
            0, ch, 
            *width, *height,
            0, ch, GL_UNSIGNED_BYTE, image);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    return tex;
}

float lerp(float x, float y, float t) {
    return x + t * (y - x);
}

struct color_t leaf_color_lerp(struct color_t x, struct color_t y, float t) {

    float ir = lerp((float)x.r, (float)y.r, t);
    float ig = lerp((float)x.g, (float)y.g, t);
    float ib = lerp((float)x.b, (float)y.b, t);

    // Clamp values.
    ir = (ir < 0.0f) ? 0.0f : (ir > (float)UINT8_MAX) ? (float)UINT8_MAX : ir;
    ig = (ig < 0.0f) ? 0.0f : (ig > (float)UINT8_MAX) ? (float)UINT8_MAX : ig;
    ib = (ib < 0.0f) ? 0.0f : (ib > (float)UINT8_MAX) ? (float)UINT8_MAX : ib;

    return (struct color_t) { (uint8_t)ir, (uint8_t)ig, (uint8_t)ib };
}

struct color_t hexrgb_to_color_type(int hexrgb) {
    return (struct color_t) {
        (hexrgb & 0xFF0000) >> 16,
        (hexrgb & 0x00FF00) >> 8,
        (hexrgb & 0x0000FF)
    };
}

bool leaf_create_framebuffer(struct framebuffer* fb, uint32_t width, uint32_t height) {
    fb->fbo = 0;
    fb->texture = 0;
    fb->width = 0;
    fb->height = 0;
    glGenFramebuffers(1, &fb->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    glGenTextures(1, &fb->texture);
    glBindTexture(GL_TEXTURE_2D, fb->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach texture to framebuffer object.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->texture, 0);
    bool result = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(!result) {
        leaf_free_framebuffer(fb);
        fprintf(stderr, "%s: Failed to create framebuffer (%ix%i), glError = 0x%02X\n",
                __func__, width, height, glGetError());
    }
    else {
        fb->width = width;
        fb->height = height;
    }

    return result;
}


void leaf_free_framebuffer(struct framebuffer* fb) {
    glDeleteFramebuffers(1, &fb->fbo);
    glDeleteTextures(1, &fb->texture);
}

void leaf_use_framebuffer(struct framebuffer* fb) {
    if(fb == NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glViewport(0, 0, fb->width, fb->height);
}
