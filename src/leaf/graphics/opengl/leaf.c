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
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../leaf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb_image.h"


#include "shaders.h"


static struct {
    GLFWwindow* glfw_win;
    
    size_t   renderer_num_vertex_positions;
    size_t   renderer_vbo_memsize;
    size_t   renderer_vbo_size;
    size_t   renderer_vbo_data_offset;
    size_t   renderer_vbo_num_vertices;

    uint32_t renderer_vao;
    uint32_t renderer_vbo;
    uint32_t renderer_shader;

    uint32_t renderer_tex_vao;
    uint32_t renderer_tex_vbo;
    uint32_t renderer_tex_shader;

}
g_lf = {
    .glfw_win = NULL,
    .renderer_num_vertex_positions = 0,
    .renderer_vbo_memsize = 0,
    .renderer_vbo_size = 0,
    .renderer_vbo_data_offset = 0,
    .renderer_vbo_num_vertices = 0,
    .renderer_vao = 0,
    .renderer_vbo = 0,
    .renderer_shader = 0,
    .renderer_tex_vao = 0,
    .renderer_tex_vbo = 0,
    .renderer_tex_shader = 0
};

uint32_t p_leaf_get_renderer_tex_shader() {
    return g_lf.renderer_tex_shader;
}

uint32_t p_leaf_get_renderer_tex_vao() {
    return g_lf.renderer_tex_vao;
}

uint32_t p_leaf_get_renderer_tex_vbo() {
    return g_lf.renderer_tex_vbo;
}

bool leaf_should_quit() {
    return glfwWindowShouldClose(g_lf.glfw_win);
}

void leaf_swap_buffers() {
    glfwSwapBuffers(g_lf.glfw_win);
}

void leaf_get_events() {
    glfwPollEvents();
}

void leaf_set_viewport(int x, int y, int w, int h) {
    glViewport(x, y, w, h);
}

void leaf_hide_mouse(bool is_hidden) {
    glfwSetInputMode(g_lf.glfw_win, GLFW_CURSOR, is_hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

bool leaf_key_down(int key) {
    return glfwGetKey(g_lf.glfw_win, key) == GLFW_PRESS;
}

void leaf_enable_vsync(bool is_enabled) {
    glfwSwapInterval(is_enabled ? 0 : 1);
}

double leaf_get_time_insec() {
    return glfwGetTime();
}

void leaf_clear_color(RGBAColor color) {
    glClearColor
    (
        (float)color.r / 255.0f,
        (float)color.g / 255.0f,
        (float)color.b / 255.0f,
        (float)color.a / 255.0f
    );
}

void leaf_clear() {
    glClear(GL_COLOR_BUFFER_BIT);
}

void leaf_enable_scissor_test(bool is_enabled) {
    if(is_enabled) {
        glEnable(GL_SCISSOR_TEST);
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void leaf_set_scissor_region(int x, int y, int w, int h) {
    glScissor(x, y, w, h);
}



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


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_RELEASE) {
        return;
    }
    
    LeafCtx* ctx = (LeafCtx*)glfwGetWindowUserPointer(window);
    if(ctx->callback.key_pressed == NULL) {
        return;
    }

    ctx->callback.key_pressed(ctx->callback.user_pointer, key, mods);
}

void glfw_char_callback(GLFWwindow* window, uint32_t codepoint) {
    LeafCtx* ctx = (LeafCtx*)glfwGetWindowUserPointer(window);
    if(ctx->callback.char_pressed == NULL) {
        return;
    }
        
    ctx->callback.char_pressed(ctx->callback.user_pointer, codepoint);
}

void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    LeafCtx* ctx = (LeafCtx*)glfwGetWindowUserPointer(window);
    if(ctx->callback.window_resized == NULL) {
        return;
    }
        
    ctx->callback.window_resized(ctx->callback.user_pointer, width, height);
}

//void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
//void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
//void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

LeafCtx* leaf_open(const char* title, int width, int height, int flags) {
    LeafCtx* ctx = malloc(sizeof *ctx);
    if(!ctx) {
        goto out;
    }

    ctx->callback.key_pressed = NULL;
    ctx->callback.char_pressed = NULL;
    ctx->callback.window_resized = NULL;

    g_lf.glfw_win = NULL;
    //g_lf.glfw_win = NULL;
    ctx->win_width = width;
    ctx->win_height = height;

    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialzie GLFW\n");
        goto out;
    }


    if(flags & LEAF_NORESIZE) {
        glfwWindowHint(GLFW_RESIZABLE, false);
    }

    g_lf.glfw_win = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!g_lf.glfw_win) {
        fprintf(stderr, "Failed to create window\n");
        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }

    if(flags & LEAF_NORESIZE) {
        glfwSetWindowSizeLimits(g_lf.glfw_win, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
    else {
        glfwSetWindowSizeLimits(g_lf.glfw_win, 500, 500, 6000, 3000);
    }

    glfwMakeContextCurrent(g_lf.glfw_win);
    
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

    glfwSetWindowUserPointer  (g_lf.glfw_win, ctx);
    glfwSetInputMode          (g_lf.glfw_win, GLFW_STICKY_KEYS, GLFW_FALSE);
    glfwSetKeyCallback        (g_lf.glfw_win, glfw_key_callback);
    glfwSetCharCallback       (g_lf.glfw_win, glfw_char_callback);
    glfwSetWindowSizeCallback (g_lf.glfw_win, glfw_window_resize_callback);

    //glfwSetScrollCallback     (g_lf.glfw_win, glfw_scroll_callback);
    //glfwSetCursorPosCallback  (g_lf.glfw_win, glfw_cursor_position_callback);
    //glfwSetMouseButtonCallback(g_lf.glfw_win, glfw_mouse_button_callback);
    
    /*
    renderer_vao = 0;
    renderer_vbo = 0;
    renderer_shader = 0;
    renderer_num_vertex_positions = 0;
    renderer_vbo_memsize = 0;
    renderer_vbo_size = 0;
    */
out:
    return ctx;
}


void leaf_quit(LeafCtx* ctx) {
    if(!ctx) {
        return;
    }

    if(g_lf.glfw_win) {
        glfwDestroyWindow(g_lf.glfw_win);
    }

    glfwTerminate();
    free(ctx);
}

void leaf_init_renderer(LeafCtx* ctx, size_t vbo_memsize) {    
    glGenVertexArrays(1, &g_lf.renderer_vao);
    glBindVertexArray(g_lf.renderer_vao);

    glGenBuffers(1, &g_lf.renderer_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_lf.renderer_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_memsize, NULL, GL_STATIC_DRAW);

    g_lf.renderer_vbo_memsize = vbo_memsize;
    g_lf.renderer_vbo_data_offset = 0;
    g_lf.renderer_vbo_num_vertices = 0;
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
 
    glGenVertexArrays(1, &g_lf.renderer_tex_vao);
    glBindVertexArray(g_lf.renderer_tex_vao);

    glGenBuffers(1, &g_lf.renderer_tex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_lf.renderer_tex_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_memsize, NULL, GL_STATIC_DRAW);


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
    g_lf.renderer_shader = create_shader_program
        (RENDERER_VERTEX_SHADER_SRC, RENDERER_FRAGMENT_SHADER_SRC);

    // Compile rendering shaders.
    g_lf.renderer_tex_shader = create_shader_program
        (RENDERER_TEX_VERTEX_SHADER_SRC, RENDERER_TEX_FRAGMENT_SHADER_SRC);
}

void leaf_free_renderer(LeafCtx* ctx) {
    if(g_lf.renderer_vao > 0) {
        glDeleteVertexArrays(1, &g_lf.renderer_vao);
        g_lf.renderer_vao = 0;
    }
    if(g_lf.renderer_vbo > 0) {
        glDeleteBuffers(1, &g_lf.renderer_vbo);
        g_lf.renderer_vbo = 0;
    }
    if(g_lf.renderer_shader > 0) {
        glDeleteProgram(g_lf.renderer_shader);
        g_lf.renderer_shader = 0;
    }
}

void leaf_render_vertices(LeafCtx* ctx, float* vertices, size_t vertices_memsize) {   
    bool divisible = !(vertices_memsize % 5); // 2(x, y) + 3(r, g, b) = 5
    if(!divisible) {
        fprintf(stderr, "(%s) %s(): Vertex data format is incorrect.\n",
                __FILE__, __func__);
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, g_lf.renderer_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_memsize, vertices);
    
    const int num_vertex_positions = ((vertices_memsize / sizeof(float)) / 5) * 2;
   
    glBindVertexArray(g_lf.renderer_vao);
   
    glUseProgram(g_lf.renderer_shader);
    glDrawArrays(GL_TRIANGLES, 0, num_vertex_positions);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void leaf_renderer_flush(LeafCtx* ctx) {

    glBindBuffer(GL_ARRAY_BUFFER, g_lf.renderer_vbo);
    glBindVertexArray(g_lf.renderer_vao);
    glUseProgram(g_lf.renderer_shader);

    glDrawArrays(GL_TRIANGLES, 0, g_lf.renderer_vbo_num_vertices);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    g_lf.renderer_vbo_data_offset = 0;
    g_lf.renderer_vbo_num_vertices = 0;
}

LeafTexture leaf_load_texture(const char* path) {
    int width = 0;
    int height = 0;
    uint32_t tex = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);
    uint8_t* image = stbi_load(path, &width, &height, &channels, 0);

    printf("%s(): %s: %i, %i\n", __func__, path, width, height);
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
            width, height,
            0, ch, GL_UNSIGNED_BYTE, image);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    return (LeafTexture) {
        .id = tex,
        .width = width,
        .height = height
    };
}



bool leaf_create_framebuffer(LeafFramebuffer* fb, uint32_t width, uint32_t height) {
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


void leaf_free_framebuffer(LeafFramebuffer* fb) {
    glDeleteFramebuffers(1, &fb->fbo);
    glDeleteTextures(1, &fb->texture);
}

void leaf_use_framebuffer(LeafFramebuffer* fb) {
    if(fb == NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glViewport(0, 0, fb->width, fb->height);
}
