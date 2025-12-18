#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "nemi.h"
#include "nemi_config.h"
#include "common.h"
#include "plscript_funcs.h"


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_window_resize_callback(GLFWwindow* window, int width, int height);
void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);;
void glfw_char_callback(GLFWwindow* window, uint32_t codepoint);




struct nemi* start_session() {
    struct nemi* st = malloc(sizeof *st);

    st->flags = 0;
    st->lfctx = leaf_open("Nemi - Terminal Emulator", 900, 700);
    st->num_terminals = 0;


    zero_input_buffers(st);
    //init_default_palette(st);
    init_default_config(st);

    if(!leaf_load_font(&st->font, "Topaz-8.ttf")) {
        quit_session(st);
        return NULL;
    }
    
    st->flags |= FLG_FONT_LOADED;

    st->font.center_char_to_cell = true;
    st->font.spacing = 0.2f;
    leaf_set_font_scale(&st->font, 0.9);
    leaf_set_font_color(&st->font, 1.0, 0.8, 0.6);

    glfwSetWindowUserPointer(st->lfctx->glfw_win, st);
    glfwSetKeyCallback        (st->lfctx->glfw_win, glfw_key_callback);
    glfwSetCharCallback       (st->lfctx->glfw_win, glfw_char_callback);
    glfwSetScrollCallback     (st->lfctx->glfw_win, glfw_scroll_callback);
    glfwSetWindowSizeCallback (st->lfctx->glfw_win, glfw_window_resize_callback);
    
    const size_t renderer_memory_size = 1024 * sizeof(float);
    leaf_init_renderer(st->lfctx, renderer_memory_size);
    

    st->win_cols = st->lfctx->win_width / st->font.char_width;
    st->win_rows = st->lfctx->win_height / (st->font.char_height + st->cfg.line_padding);
    st->win_cols -= 1;
    st->win_rows -= 1;

    // Spawn default terminal.
    st->terminal = spawn_terminal(st, st->win_rows, st->win_cols);

    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        st->scripts[i] = (struct perl_script) {
            .perl_interp = NULL,
            .is_loaded   = false
        };
    }
   
    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        st->renderbufs[i] = (struct render_buffer) {
            .nodes = NULL,
            .num_nodes_max = 0,
            .num_nodes = 0
        };
    }

    plscript_funcs_set_context(st);


    if(!nemi_read_config(st, "nemi.ini")) {
        fprintf(stderr, "Error occurred while reading config file.\n");
        free(st);
        return NULL;
    }



    return st;
}

void quit_session(struct nemi* st) {
    if((st->flags & FLG_FONT_LOADED)) {
        leaf_unload_font(&st->font);
    }

    for(uint16_t i = 0; i < st->num_terminals; i++) {
        close_terminal(&st->terminals[i]);
    }

    leaf_free_renderer(st->lfctx);
    leaf_quit(st->lfctx);


    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        nemi_unload_perl_script(&st->scripts[i]);
    }

    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        freeif(st->renderbufs[i].nodes);
    }


    log_close();
    freeif(st);
}

void zero_input_buffers(struct nemi* st) {
    for(int i = 0; i < NEMI_KEYINBUF_MAX; i++) {
        st->key_inputs[i] = 0;
    }
    for(int i = 0; i < NEMI_CHARINBUF_MAX; i++) {
        st->char_inputs[i] = 0;
    }
}

void begin_frame(struct nemi* st) {
    glClearColor(
            (float)st->cfg.colors[NEMI_COLOR_BG].r / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].g / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].b / 255.0f,
            1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void end_frame(struct nemi* st) {


    // TODO: Optimize this later.
    // --------------------------
    /*
    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        struct render_buffer* rb = &st->renderbufs[i];
        if(!rb->meshes) {
            continue;
        }

        for(size_t j = 0; j < rb->num_meshes_max; j++) {
            struct vertex_mesh* mesh = &rb->meshes[j];
            if(!mesh->vertices) {
                continue;
            }

            leaf_render_vertices(st->lfctx, mesh->vertices, mesh->vertices_memsize);
        }
    }
    */

    st->last_char_in = 0;
    st->last_key_in = 0;
    glfwSwapBuffers(st->lfctx->glfw_win);
    glfwPollEvents();
}

bool key_down(struct nemi* st, int key) {
    return (glfwGetKey(st->lfctx->glfw_win, key) == GLFW_PRESS);
}

void init_default_config(struct nemi* st) {
    st->cfg.padding_x = 10;
    st->cfg.padding_y = 10;
    st->cfg.line_padding = 3;
    
    int d_v = 180;
    int b_v = 255;
    int l_v = 30;

    st->cfg.colors[NEMI_COLOR_FG] = (struct color_t){ 200, 180, 160 };
    st->cfg.colors[NEMI_COLOR_BG] = (struct color_t){ 10, 10, 10 };
    st->cfg.colors[NEMI_COLOR_BLACK]   = (struct color_t){ 0, 0, 0 };
    st->cfg.colors[NEMI_COLOR_RED]     = (struct color_t){ d_v, l_v, l_v };
    st->cfg.colors[NEMI_COLOR_GREEN]   = (struct color_t){ l_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_YELLOW]  = (struct color_t){ d_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_BLUE]    = (struct color_t){ l_v + 40, l_v + 40, d_v };
    st->cfg.colors[NEMI_COLOR_MAGENTA] = (struct color_t){ d_v, l_v, d_v };
    st->cfg.colors[NEMI_COLOR_CYAN]    = (struct color_t){ l_v, d_v, d_v };
    st->cfg.colors[NEMI_COLOR_WHITE]   = (struct color_t){ d_v, d_v, d_v };

    st->cfg.colors[NEMI_BRIGHT_COLOR_BLACK]   = (struct color_t){ 10, 10, 10 };
    st->cfg.colors[NEMI_BRIGHT_COLOR_RED]     = (struct color_t){ b_v, l_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_GREEN]   = (struct color_t){ l_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_YELLOW]  = (struct color_t){ b_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_BLUE]    = (struct color_t){ l_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_MAGENTA] = (struct color_t){ b_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_CYAN]    = (struct color_t){ l_v, b_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_WHITE]   = (struct color_t){ b_v, b_v, b_v };

}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    if((action != GLFW_PRESS)
    && (action != GLFW_REPEAT)) {
        return;
    }

    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
    push_key_input(st, key);
    st->last_key_in = key;
    
    /*
    if(key == GLFW_KEY_SPACE) {
        renderbuf_test(st);
    }
    */

    terminal_handle_key_event(st, st->terminal);
}

void glfw_char_callback(GLFWwindow* window, uint32_t codepoint) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
    if(codepoint >= 0x20 && codepoint <= 0x7E) {
        push_char_input(st, codepoint);
        st->last_char_in = codepoint;
    
        terminal_handle_char_event(st, st->terminal);
    }
}

void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    (void)x_offset;
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
}

void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);

    st->lfctx->win_width = width;
    st->lfctx->win_height = height;

    glViewport(0, 0, width, height);

    st->win_rows = st->lfctx->win_width / st->font.char_width;
    st->win_cols = st->lfctx->win_height / (st->font.char_height + st->cfg.line_padding);

    st->win_cols -= 1;
    st->win_rows -= 1;

    for(size_t i = 0; i < st->num_terminals; i++) {
        terminal_handle_resize_event(st, &st->terminals[i]);
    }
}

void push_key_input(struct nemi* st, int key) {
    for(int i = NEMI_KEYINBUF_MAX-1; i > 0; i--) {
        st->key_inputs[i] = st->key_inputs[i-1];
    }
    st->key_inputs[0] = key;
}

void push_char_input(struct nemi* st, char ch) {
    for(int i = NEMI_CHARINBUF_MAX-1; i > 0; i--) {
        st->char_inputs[i] = st->char_inputs[i-1];
    }
    st->char_inputs[0] = ch;
}

int coltox(struct nemi* st, int col) {
    return col * st->font.char_width + st->cfg.padding_x;
}

int rowtoy(struct nemi* st, int row) {
    return row * (st->font.char_height + st->cfg.line_padding) + st->cfg.padding_y;
}

int new_renderbuf(struct nemi* st, int num_nodes_max) {
    int ret_index = -1;

    // Find unused one and return the index.
    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        struct render_buffer* rb = &st->renderbufs[i];
        if(rb->nodes) {
            continue;
        }

        rb->nodes = calloc(sizeof *rb->nodes, num_nodes_max);
        rb->num_nodes_max = num_nodes_max;
        rb->num_nodes     = 0;
        logprintf(LOG_INFO, "Created new render buffer with %i nodes.", num_nodes_max);

        ret_index = i;
        break;
    }

    return ret_index;
}

