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


#ifndef LEAF_H
#define LEAF_H


#include "draw.h"
#include "texture.h"
#include "common.h"
#include "framebuffer.h"
//#include <GLFW/glfw3.h>






typedef struct LeafCtx_t {

    struct {
        void(*key_pressed)    (void* user_ptr, int key, int mods);
        void(*char_pressed)   (void* user_ptr, uint32_t codepoint);
        void(*window_resized) (void* user_ptr, int width, int height);
    
        void* user_pointer;
    }
    callback;
        
    int win_width;
    int win_height;
}
LeafCtx;

// 'flags' for leaf_open()
#define LEAF_NORESIZE (1 << 0)



// The graphics backend needs to implement these.

LeafCtx* leaf_open (const char* title_or_device, int width, int height, int flags);
void     leaf_quit (LeafCtx* ctx);

bool     leaf_should_quit();
void     leaf_swap_buffers();
void     leaf_get_events(LeafCtx* ctx);
void     leaf_set_viewport(int x, int y, int w, int h);
void     leaf_hide_mouse(bool is_hidden);
void     leaf_enable_vsync(bool is_enabled);
bool     leaf_key_down(int key); // see 'keyboard.h' for keys.
double   leaf_get_time_insec();

void     leaf_clear_color  (RGBAColor color);
void     leaf_clear        ();

// Scissor test is same as OpenGL's GL_SCISSOR_TEST.
void     leaf_enable_scissor_test (bool is_enabled);
void     leaf_set_scissor_region  (int x, int y, int w, int h);



// Normalize coordinates from (0 <-> win_width/height) to (-1.0 <-> +1.0)
// TODO: Maybe this should be renamed because this is for OpenGL.
//void leaf_normalize_xy_to_ndc  (float x_in, float y_in, float* x_out, float* y_out);

// OpenGL framebuffer control.
bool leaf_create_framebuffer   (LeafFramebuffer* fb, uint32_t width, uint32_t height);
void leaf_use_framebuffer      (LeafFramebuffer* fb);
void leaf_free_framebuffer     (LeafFramebuffer* fb);

LeafTexture leaf_load_texture  (const char* path);


#ifdef GRAPHICS_OPENGL

// Renderer must be initialized if OpenGL
void leaf_init_renderer        (LeafCtx* ctx, size_t vbo_memsize);
void leaf_free_renderer        (LeafCtx* ctx);

// One vertex is: [x, y, r, g, b]
void leaf_render_vertices      (LeafCtx* ctx, float* vertices, size_t vertices_memsize);
void leaf_renderer_flush       (LeafCtx* ctx);

// graphics/opengl/draw.c needs these.
uint32_t p_leaf_get_renderer_tex_shader();
uint32_t p_leaf_get_renderer_tex_vao();
uint32_t p_leaf_get_renderer_tex_vbo();

#endif // GRAPHICS_OPENGL


#ifdef GRAPHICS_LINUX_FBDEV

void p_leaf_set_activefb_pixel(size_t pixel_index, RGBColor color);
void p_leaf_set_activefb_pixel_xy(int x, int y, RGBColor color);
void p_leaf_draw_framebuffer(LeafFramebuffer* fb);

#endif // GRAPHICS_LINUX_FBDEV




#endif
