#ifndef LEAF_H
#define LEAF_H

#include <GLFW/glfw3.h>


#include "draw.h"



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
    int win_width;
    int win_height;

    uint32_t renderer_vao;
    uint32_t renderer_vbo;
    uint32_t renderer_shader;

    uint32_t renderer_tex_vao;
    uint32_t renderer_tex_vbo;
    uint32_t renderer_tex_shader;

    size_t   renderer_num_vertex_positions;
    size_t   renderer_vbo_memsize;
    size_t   renderer_vbo_size;
    size_t   renderer_vbo_data_offset;
    size_t   renderer_vbo_num_vertices;
}
LeafCtx;

// 'flags' for leaf_open()
#define LEAF_NORESIZE (1 << 0)


LeafCtx* leaf_open (const char* title, int width, int height, int flags);
void     leaf_quit (LeafCtx* ctx);

// Normalize coordinates from (0 <-> win_width/height) to (-1.0 <-> +1.0)
void leaf_normalize_coords     (float x_in, float y_in, float* x_out, float* y_out);

void leaf_init_renderer        (LeafCtx* ctx, size_t vbo_memsize);
void leaf_free_renderer        (LeafCtx* ctx);

//void leaf_render_vertices(LeafCtx* ctx);
//void leaf_clear_vertices(LeafCtx* ctx);

// One vertex: [x, y, r, g, b]
void leaf_render_vertices      (LeafCtx* ctx, float* vertices, size_t vertices_memsize);
void leaf_renderer_flush       (LeafCtx* ctx);
uint32_t leaf_load_texture     (const char* path, int* width, int* height);


RGBColor hexrgb_to_color_type (int hexrgb);
float leaf_lerp               (float x, float y, float t);
RGBColor leaf_color_lerp      (RGBColor x, RGBColor y, float t);

void leaf_use_framebuffer     (LeafFramebuffer* fb);
bool leaf_create_framebuffer  (LeafFramebuffer* fb, uint32_t width, uint32_t height);
void leaf_free_framebuffer    (LeafFramebuffer* fb);

#endif
