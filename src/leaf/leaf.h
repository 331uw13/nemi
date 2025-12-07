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

    size_t   renderer_num_vertex_positions;
    size_t   renderer_vbo_memsize;
    size_t   renderer_vbo_size;
};


struct leaf_ctx_t* leaf_open(const char* title, int width, int height);
void               leaf_quit(struct leaf_ctx_t* ctx);

// Normalize coordinates from (0 <-> win_width/height) to (-1.0 <-> +1.0)
void leaf_normalize_coords(float x_in, float y_in, float* x_out, float* y_out);

void leaf_init_renderer(struct leaf_ctx_t* ctx, size_t max_num_vertices);
void leaf_free_renderer(struct leaf_ctx_t* ctx);

//void leaf_render_vertices(struct leaf_ctx_t* ctx);
//void leaf_clear_vertices(struct leaf_ctx_t* ctx);

// One vertex: [x, y, r, g, b]
void leaf_render_vertices(struct leaf_ctx_t* ctx, float* vertices, size_t num_vertices);


#endif
