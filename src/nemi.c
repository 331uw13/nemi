#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "nemi.h"
#include "nemi_config.h"
#include "common.h"

static struct nemi* g_nemi_state = NULL;


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_window_resize_callback(GLFWwindow* window, int width, int height);
void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);;
void glfw_char_callback(GLFWwindow* window, uint32_t codepoint);

struct nemi* get_state() {
    return g_nemi_state;
}

struct nemi* start_session(const char* config_file) {
    struct nemi* st = malloc(sizeof *st);

    st->frame_time = 0.001;
    st->frame_time_begin = 0.0;
    st->flags = 0;
    st->lfctx = leaf_open("Nemi - Terminal Emulator", 900, 700);
    st->num_terminals = 0;


    zero_input_buffers(st);
    init_default_config(st);

    struct nemi_font_config font_cfg;
    nemi_read_font_config(st, config_file, &font_cfg);

    if(!leaf_load_font(&st->font, font_cfg.font_filepath)) {
        quit_session(st);
        return NULL;
    }
    
    st->flags |= FLG_FONT_LOADED;

    st->font.center_char_to_cell = font_cfg.font_center_char_to_cell;
    st->font.spacing = 0.2f;
    leaf_set_font_scale(&st->font, 0.9);


    leaf_set_font_color(&st->font, (struct color_t) { 255, 200, 150 });


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

    //plscript_funcs_set_context(st);
    g_nemi_state = st;


    if(!nemi_read_config(st, config_file)) {
        fprintf(stderr, "Error occurred while reading config file.\n");
        free(st);
        return NULL;
    }

    if(st->cfg.hide_mouse) {
        glfwSetInputMode(st->lfctx->glfw_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }


    glfwSwapInterval(st->cfg.vsync ? 0 : 1);


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
        unload_perl_script(&st->scripts[i]);
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


static
void render_rbbuffer_nodes(struct nemi* st, enum rb_node_layer layer) {

    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        struct render_buffer* rb = &st->renderbufs[i];
        if(rb->num_nodes == 0) {
            continue;
        }

        struct rb_node* rnode = &rb->nodes[0];
        
        while(rnode) {

//        for(size_t i = 0; i < rb->num_nodes; i++) {
//            rnode = &rb->nodes[i];

            if(rnode->hidden || (rnode->layer != layer)) {
                rnode = rnode->next;
                continue;
            }
            
            switch(rnode->type) {
                case RBNODE_UNUSED: 
                    logprintf(LOG_WARN, "Trying to render unused rbnode.");
                    break; // Ignored.
                   
                case RBNODE_MESH:
                    leaf_render_vertices(st->lfctx, 
                            rnode->mesh.vertices, rnode->mesh.vertices_memsize);
                    
                    break;

                case RBNODE_TEXT:
                    leaf_set_font_color(&st->font, rnode->text.color);

                    leaf_draw_text(&st->font, 
                            rnode->text.pos_x, rnode->text.pos_y,
                            rnode->text.data,
                            rnode->text.len);
                    break;

                default:
                    logprintf(LOG_ERROR, "Unknown rbnode type %i", rnode->type);
                    break;


                // More will be added later.
            }

            rnode = rnode->next;
        }
    }
}

void begin_frame(struct nemi* st) {
    st->frame_time_begin = glfwGetTime();
    glClearColor(
            (float)st->cfg.colors[NEMI_COLOR_BG].r / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].g / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].b / 255.0f,
            1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void end_frame(struct nemi* st) {

    if(st->cfg.show_frametime) {
        float old_font_scale = st->font.scale;
        leaf_set_font_scale(&st->font, 0.7f);
        leaf_draw_text_fmt(&st->font, 
                st->lfctx->win_width-250, 10,
                "frame_time=%0.3fms", st->frame_time * 1000.0);
        leaf_set_font_scale(&st->font, old_font_scale);
    }
   

    render_rbbuffer_nodes(st, RBNODE_LAYER_LAST);
    render_rbbuffer_nodes(st, RBNODE_LAYER_FIRST);
    leaf_font_render(st->lfctx, &st->font);


    st->last_char_in = 0;
    st->last_key_in = 0;
    glfwSwapBuffers(st->lfctx->glfw_win);
    glfwPollEvents();
   
    usleep(10 * 1000);


    st->frame_time = glfwGetTime() - st->frame_time_begin;
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
    st->cfg.colors[NEMI_COLOR_BG] = (struct color_t){ 12, 13, 15 };
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


// 'event_num' corresponds to REG_EVENT... defined in "script.h"
void trigger_event_for_scripts(struct nemi* st, int event_num, 
        const char* arg_types, ...) {
    
    size_t num_args = strlen(arg_types);

    char* func_args[num_args+1];
    func_args[num_args] = NULL;
        
    char args_str[num_args][16];


    if(num_args > 0) {

        for(size_t i = 0; i < num_args; i++){
            memset(args_str[i], 0, sizeof(args_str[i]));
        }

        va_list args;
        va_start(args, arg_types);
        for(size_t i = 0; i < num_args; i++) {
            char* buf = args_str[i];
            size_t buf_memsize = sizeof(args_str[i])-1;
            
            switch(arg_types[i]) {
                case 'i':
                    snprintf(buf, buf_memsize, "%d", va_arg(args, int));
                    break;

                case 'f':
                    snprintf(buf, buf_memsize, "%f", va_arg(args, double));
                    break;

            }

            func_args[i] = args_str[i];
        }

        va_end(args);
    }

    const char* event_name = plscript_get_event_name(event_num);

    for(size_t i = 0; i < NEMI_SCRIPTS_MAX; i++) {
        struct perl_script* script = &st->scripts[i];
        if(!script->is_loaded) {
            continue;
        }

        if(!(script->reg_events & event_num)) {
            continue;
        }
        
        if(num_args > 0) {
            plscript_call_args(script, event_name, func_args);
        }
        else {
            plscript_call(script, event_name);
        }
    }
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;

    if(action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
    push_key_input(st, key);

    st->last_key_in = key;
    st->last_keymod_in = mods;
    

    trigger_event_for_scripts(st, REG_EVENT_KEY_INPUT,
            "ii",
            key, 
            mods);

    if(mods == GLFW_MOD_CONTROL) {
        if(key == GLFW_KEY_1) {
            font_scale(st, -0.1);
        }
        if(key == GLFW_KEY_2) {
            font_scale(st, +0.1);
        }
    }
    terminal_handle_key_event(st, st->terminal);
}

void glfw_char_callback(GLFWwindow* window, uint32_t codepoint) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);

    if(codepoint >= 0x20 && codepoint <= 0x7E) {
        trigger_event_for_scripts(st, REG_EVENT_CHAR_INPUT,
            "i",
            codepoint);

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

    st->win_cols = width / st->font.char_width;
    st->win_rows = height / (st->font.char_height + st->cfg.line_padding);

    st->win_cols -= 1;
    st->win_rows -= 1;

    for(size_t i = 0; i < st->num_terminals; i++) {
        terminal_handle_resize_event(st, &st->terminals[i]);
    }

    trigger_event_for_scripts(st, REG_EVENT_WIN_RESIZED,
            "ii",
            st->win_cols,
            st->win_rows);
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

    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        struct render_buffer* rb = &st->renderbufs[i];
        if(rb->nodes) {
            continue;
        }

        rb->nodes = calloc(num_nodes_max, sizeof *rb->nodes);
        rb->num_nodes_max = num_nodes_max;
        rb->num_nodes     = 0;
        logprintf(LOG_INFO, "Created new render buffer with %i nodes.", num_nodes_max);

        ret_index = i;
        break;
    }

    return ret_index;
}

void font_scale(struct nemi* st, float offset) {
    leaf_set_font_scale(&st->font, st->font.scale + offset);

    // We can call glfw window resize callback here
    // it resizes the terminals rows and columns too.
    glfw_window_resize_callback(st->lfctx->glfw_win, st->lfctx->win_width, st->lfctx->win_height);
}

void set_font_scale(struct nemi* st, float scale) {
    leaf_set_font_scale(&st->font, scale);
    glfw_window_resize_callback(st->lfctx->glfw_win, st->lfctx->win_width, st->lfctx->win_height);
}

