#ifndef LEAF_H
#define LEAF_H

#include <GLFW/glfw3.h>


#include "draw.h"

struct leaf_ctx_t {
    GLFWwindow* glfw_win;
    int win_width;
    int win_height;
    struct font_t font;


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
};

// 'flags' for leaf_open()
#define LEAF_NORESIZE (1 << 0)


struct leaf_ctx_t* leaf_open(const char* title, int width, int height, int flags);
void               leaf_quit(struct leaf_ctx_t* ctx);

// Normalize coordinates from (0 <-> win_width/height) to (-1.0 <-> +1.0)
void leaf_normalize_coords(float x_in, float y_in, float* x_out, float* y_out);

void leaf_init_renderer(struct leaf_ctx_t* ctx, size_t vbo_memsize);
void leaf_free_renderer(struct leaf_ctx_t* ctx);

//void leaf_render_vertices(struct leaf_ctx_t* ctx);
//void leaf_clear_vertices(struct leaf_ctx_t* ctx);

// One vertex: [x, y, r, g, b]
void leaf_render_vertices(struct leaf_ctx_t* ctx, float* vertices, size_t vertices_memsize);
void leaf_renderer_flush(struct leaf_ctx_t* ctx);
uint32_t leaf_load_texture(const char* path, int* width, int* height);


struct color_t hexrgb_to_color_type(int hexrgb);

float leaf_lerp(float x, float y, float t);
struct color_t leaf_color_lerp(struct color_t x, struct color_t y, float t);

#endif
