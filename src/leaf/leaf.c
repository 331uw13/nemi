#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "leaf.h"
#include "shaders.h"

static const char RENDERER_VERTEX_SHADER_SRC[] = {
    "#version 460 core\n"
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
    "#version 460 core\n"    
    "out vec4 out_color;\n"  
    "in vec3 v_color;\n"
    "\n"
    "void main() {\n"
    "    out_color = vec4(v_color, 1.0);\n"
    "}\n",
};



struct leaf_ctx_t* leaf_open(const char* title, int width, int height) {
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


    glfwWindowHint(GLFW_RESIZABLE, false); // TODO: Make this optional.

    ctx->glfw_win = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!ctx->glfw_win) {
        fprintf(stderr, "Failed to create window\n");
        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }

    glfwSetWindowSizeLimits(ctx->glfw_win, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
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


    const size_t stride_size = (2 + 3) * sizeof(float);
    // Positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_size, (void*)(0));
    glEnableVertexAttribArray(0);

    // Colors.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride_size, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Compile rendering shaders.
    ctx->renderer_shader = create_shader_program
        (RENDERER_VERTEX_SHADER_SRC, RENDERER_FRAGMENT_SHADER_SRC);

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

void leaf_render_vertices(struct leaf_ctx_t* ctx, float* vertices, size_t vertices_sizeb) {
   
    bool divisible = !(vertices_sizeb % 5); // 2(x, y) + 3(r, g, b) = 5
    if(!divisible) {
        fprintf(stderr, "(%s) %s(): Vertex data format is incorrect.\n",
                __FILE__, __func__);
        return;
    }

    if(vertices_sizeb >= ctx->renderer_vbo_memsize) {
        fprintf(stderr, "(%s) %s(): Renderer VBO doesnt have enough memory allocated.\n",
                __FILE__, __func__);
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_sizeb, vertices);
    
    const int num_vertex_positions = ((vertices_sizeb / sizeof(float)) / 5) * 2;
   
    glBindVertexArray(ctx->renderer_vao);
   
    glUseProgram(ctx->renderer_shader);
    glDrawArrays(GL_TRIANGLES, 0, num_vertex_positions);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}




