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


/* ---------------------------------------------------------
    
    If you dont want to use a desktop environment,
    and only the linux framebuffer (/dev/fb0)
    You can compile with:
        -DGRAPHICS_LINUX_FBDEV

        This also has the effect that you
        dont need to link with GLFW, OpenGL and freetype2.

    Otherwise GLFW, OpenGL and Freetype2
    are going to be used. 
    This is the default.

------------------------------------------------------------*/

#ifndef LEAF_H
#define LEAF_H


#include "draw.h"

#include <GLFW/glfw3.h>




typedef struct LeafFramebuffer {
    uint32_t texture;
    uint32_t fbo;
    uint32_t rbo;
    uint32_t width;
    uint32_t height;
}
LeafFramebuffer;


typedef struct LeafCtx_t {
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

    struct {
        void(*key_pressed)    (void* user_ptr, int key, int mods);
        void(*char_pressed)   (void* user_ptr, uint32_t codepoint);
        void(*window_resized) (void* user_ptr, int width, int height);
    }
    callback;
    
    int win_width;
    int win_height;
}
LeafCtx;

// 'flags' for leaf_open()
#define LEAF_NORESIZE (1 << 0)


LeafCtx* leaf_open (const char* title, int width, int height, int flags);
void     leaf_quit (LeafCtx* ctx);


// Normalize coordinates from (0 <-> win_width/height) to (-1.0 <-> +1.0)
// TODO: Maybe this should be renamed because this is for OpenGL.
void leaf_normalize_xy_to_ndc  (float x_in, float y_in, float* x_out, float* y_out);

// Renderer must be initialized if OpenGL
void leaf_init_renderer        (LeafCtx* ctx, size_t vbo_memsize);
void leaf_free_renderer        (LeafCtx* ctx);

// OpenGL framebuffer control.
bool leaf_create_framebuffer   (LeafFramebuffer* fb, uint32_t width, uint32_t height);
void leaf_use_framebuffer      (LeafFramebuffer* fb);
void leaf_free_framebuffer     (LeafFramebuffer* fb);

// One vertex is: [x, y, r, g, b]
void leaf_render_vertices      (LeafCtx* ctx, float* vertices, size_t vertices_memsize);
void leaf_renderer_flush       (LeafCtx* ctx);
uint32_t leaf_load_texture     (const char* path, int* width, int* height);



RGBColor hexrgb_to_color_type (int hexrgb);
float    leaf_lerp            (float x, float y, float t);
RGBColor leaf_color_lerp      (RGBColor x, RGBColor y, float t);


#endif
