#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nemi.h"
#include "nemi_config.h"
#include "common.h"
#include "memory.h"

#include "thirdparty/stb_ds.h"



static struct nemi* g_nemi_state = NULL;


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_window_resize_callback(GLFWwindow* window, int width, int height);
void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset);;
void glfw_char_callback(GLFWwindow* window, uint32_t codepoint);

struct nemi* get_state() {
    return g_nemi_state;
}

void prepare_from_hotreload(struct nemi* st) {
    leaf_set_drawing_context(st->lfctx);
    g_nemi_state = st;
}


struct nemi* start_session(struct nemi_filepaths filepaths) {
    struct nemi* st = malloc(sizeof *st);

    st->filepaths = filepaths; 
    st->frame_time = 0.001;
    st->frame_time_begin = 0.0;
    st->flags = 0;
    st->num_scripts = 0;
    st->lfctx = leaf_open("Nemi - Terminal Emulator", 900, 700, LEAF_NORESIZE);
    st->num_terminals = 0;
    st->term_ignore_char_input_counter = 0;
    st->term_ignore_key_input_counter = 0;

    zero_input_buffers(st);
    init_default_config(st);

    if(!nemi_read_configs(st, st->filepaths.configs)) {
        logprintf(LOG_ERROR, "Failed to read configs from '%s'", st->filepaths.configs);
        quit_session(st);
        return NULL;
    }



    st->font.loaded = false;

    if(access(st->cfg.font.filepath, R_OK) == 0) {
        leaf_load_font(&st->font, st->cfg.font.filepath);
    }
    else {
        // Font was not found, Lets try from 'filepaths.fonts + font.filepath' next. 
        struct string_t font_path = string_create(0);
        string_append(&font_path, filepaths.fonts, -1);
        if(string_lastbyte(&font_path) != '/' && st->cfg.font.filepath[0] != '/') {
            string_pushbyte(&font_path, '/');
        }
        string_append(&font_path, st->cfg.font.filepath, -1);

        if(access(font_path.bytes, R_OK) == 0) {
            leaf_load_font(&st->font, font_path.bytes);
        }
        else {
            logprintf(LOG_ERROR, "Failed to find font. tried paths: '%s', '%s'",
                    st->cfg.font.filepath, font_path.bytes);
        }
        free_string(&font_path);
    }

    if(!st->font.loaded) {
        logprintf(LOG_ERROR, "Error happened while loading font.");
        quit_session(st);
        return NULL;
    }
   

    st->font.center_char_to_cell = st->cfg.font.center_char_to_cell;
    st->font.spacing = 0.2f;
    leaf_set_font_space_width(&st->font, st->font.max_bitmap_width / 2.0f);
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
    st->win_rows = st->lfctx->win_height / (st->font.char_height + st->cfg.main.line_padding);
    st->win_cols -= 1;
    st->win_rows -= 1;

    // Spawn default terminal.
    st->terminal = spawn_terminal(st, st->win_rows, st->win_cols, SHELL_TERMINAL);
    st->messages = spawn_terminal(st, st->win_rows, st->win_cols, ECHO_TERMINAL);
    st->terminal_prev = st->terminal;
    
    write_term(st->messages, TERM_WRITE_VTERM, 
            "\033[2J\033[H\033[0m\033[90m(start of messages. press ctrl+space to go back)\033[0m\n\r");

    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        st->scripts[i] = (struct perl_script) {
            .perl_interp = NULL,
            .is_loaded   = false
        };
    }
   
    /*
    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        st->renderbufs[i] = (struct render_buffer) {
            .nodes = NULL,
            .num_nodes_max = 0,
            .num_nodes = 0
        };
    }
    */

    //plscript_funcs_set_context(st);
    g_nemi_state = st;


    if(!nemi_load_scripts(st, st->filepaths.configs)) {
        logprintf(LOG_ERROR, "Failed to load all scripts.");
    }

    /*
    if(!nemi_read_config(st, config_file)) {
        fprintf(stderr, "Error occurred while reading config file.\n");
        free(st);
        return NULL;
    }

    if(st->cfg.hide_mouse) {
        glfwSetInputMode(st->lfctx->glfw_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    */
    glfwSwapInterval(st->cfg.main.vsync ? 0 : 1);

    if(st->cfg.main.hide_mouse) {
        glfwSetInputMode(st->lfctx->glfw_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
   

    leaf_create_framebuffer(&st->term_cells_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    leaf_create_framebuffer(&st->altrender_framebuffer, st->lfctx->win_width, st->lfctx->win_height);

    //clear_region(st, 0, 0, st->lfctx->win_width, st->lfctx->win_height);
    return st;
}

void quit_session(struct nemi* st) {
    leaf_unload_font(&st->font);

    for(uint16_t i = 0; i < st->num_terminals; i++) {
        close_terminal(&st->terminals[i]);
    }

    glfwSetWindowUserPointer  (st->lfctx->glfw_win, NULL);
    glfwSetKeyCallback        (st->lfctx->glfw_win, NULL);
    glfwSetCharCallback       (st->lfctx->glfw_win, NULL);
    glfwSetScrollCallback     (st->lfctx->glfw_win, NULL);
    glfwSetWindowSizeCallback (st->lfctx->glfw_win, NULL);

    leaf_free_renderer(st->lfctx);
    leaf_quit(st->lfctx);

    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        unload_perl_script(&st->scripts[i]);
    }

    leaf_free_framebuffer(&st->term_cells_framebuffer);

    /*
    for(size_t i = 0; i < ARRAY_LEN(st->renderbufs); i++) {
        freeif(st->renderbufs[i].nodes);
    }
    */

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

void switch_terminal(struct nemi* st, uint32_t index) {
    switch_terminal_ptr(st, &st->terminals[index]);
}

void switch_terminal_ptr(struct nemi* st, struct terminal* term) {
    if(st->terminal == term) {
        return;
    }
    st->terminal_prev = st->terminal;
    st->terminal = term;
    //clear_region(st, 0, 0, st->lfctx->win_width, st->lfctx->win_height);
}


void clear_color_buffer_bit(struct nemi* st) {
    glClearColor(
            (float)st->cfg.colors[NEMI_COLOR_BG].r / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].g / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].b / 255.0f,
            0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void clear_region(struct nemi* st, int x, int y, int w, int h) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, (st->lfctx->win_height - h) - y, w, h);
    clear_color_buffer_bit(st);
    glDisable(GL_SCISSOR_TEST);
}

static int frame_count = 0;



// NOTE: 'altrender_framebuffer' is active during this subroutine.
static
void altrender(struct nemi* st) {

    // Allow scripts to render stuff.
    for(uint32_t i = 0; i < st->num_scripts; i++) {
        struct perl_script* script = &st->scripts[i];
        if(script->reg_events & REG_EVENT_RENDER) {
            plscript_call(script, "event_render");
        }
    }


    leaf_draw_text_fmt(&st->font, 100, 100, "HELLO FRAMEBUFFERS!");
}

void update_frame(struct nemi* st) {
    st->frame_time_begin = glfwGetTime();


    // Render terminal cells to 'term_cells_framebuffer'
    leaf_use_framebuffer(&st->term_cells_framebuffer);
    {
        read_terminal(st, st->terminal);
        render_terminal(st, st->terminal);

        leaf_font_render(&st->font);
        leaf_renderer_flush(st->lfctx);
    }

    // Render anything else to 'altrender_framebuffer'
    leaf_use_framebuffer(&st->altrender_framebuffer);
    {
        clear_color_buffer_bit(st);
        altrender(st);
        leaf_font_render(&st->font);
        leaf_renderer_flush(st->lfctx);
    }



    leaf_use_framebuffer(NULL);
    glClearColor(
            (float)st->cfg.colors[NEMI_COLOR_BG].r / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].g / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].b / 255.0f,
            1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    leaf_draw_texture_rect(0, 0,
            st->term_cells_framebuffer.width,
            st->term_cells_framebuffer.height,
            st->term_cells_framebuffer.texture,
            (struct color_t){ 255, 255, 255 },
            LEAF_TEXTURE_NOFLIP);


    leaf_draw_texture_rect(0, 0,
            st->altrender_framebuffer.width,
            st->altrender_framebuffer.height,
            st->altrender_framebuffer.texture,
            (struct color_t){ 255, 255, 255 },
            LEAF_TEXTURE_NOFLIP);


  
    //leaf_renderer_flush(st->lfctx);
    //leaf_font_render(&st->font);

    st->last_char_in = 0;
    st->last_key_in = 0;
    glfwSwapBuffers(st->lfctx->glfw_win);
    
    glfwPollEvents();
    usleep(10000);

    st->frame_time = glfwGetTime() - st->frame_time_begin;

}

bool key_down(struct nemi* st, int key) {
    return (glfwGetKey(st->lfctx->glfw_win, key) == GLFW_PRESS);
}

void init_default_config(struct nemi* st) {
    st->cfg.main.padding_x = 10;
    st->cfg.main.padding_y = 10;
    st->cfg.main.line_padding = 3;
    
    int d_v = 180;  // Dim max.
    int b_v = 255;  // Bright max.
    int l_v = 30;   // Low value color component.

    st->cfg.colors[NEMI_COLOR_FG] = (struct color_t){ 200, 180, 160 };
    st->cfg.colors[NEMI_COLOR_BG] = (struct color_t){ 4, 4, 10 };
    st->cfg.colors[NEMI_COLOR_BLACK]   = (struct color_t){ 0, 0, 0 };
    st->cfg.colors[NEMI_COLOR_RED]     = (struct color_t){ d_v, l_v, l_v };
    st->cfg.colors[NEMI_COLOR_GREEN]   = (struct color_t){ l_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_YELLOW]  = (struct color_t){ d_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_BLUE]    = (struct color_t){ l_v + 40, l_v + 40, d_v };
    st->cfg.colors[NEMI_COLOR_MAGENTA] = (struct color_t){ d_v, l_v, d_v };
    st->cfg.colors[NEMI_COLOR_CYAN]    = (struct color_t){ l_v, d_v, d_v };
    st->cfg.colors[NEMI_COLOR_WHITE]   = (struct color_t){ d_v, d_v, d_v };

    st->cfg.colors[NEMI_BRIGHT_COLOR_BLACK]   = (struct color_t){ 70, 70, 70 };
    st->cfg.colors[NEMI_BRIGHT_COLOR_RED]     = (struct color_t){ b_v, l_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_GREEN]   = (struct color_t){ l_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_YELLOW]  = (struct color_t){ b_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_BLUE]    = (struct color_t){ l_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_MAGENTA] = (struct color_t){ b_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_CYAN]    = (struct color_t){ l_v, b_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_WHITE]   = (struct color_t){ b_v, b_v, b_v };

    st->cfg.colors[NEMI_COLOR_MESSAGES_FG] = (struct color_t){ 200, 120, 60 };
    st->cfg.colors[NEMI_COLOR_MESSAGES_BG] = (struct color_t){ 20, 10, 6 };
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
    for(size_t i = 0; i < st->num_scripts; i++) {
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
        switch(key) {
            case GLFW_KEY_1:
                font_scale(st, -0.1);
                break;

            case GLFW_KEY_2:
                font_scale(st, +0.1);
                break;
        
            case GLFW_KEY_SPACE:
                switch_terminal_ptr(st, st->terminal_prev);
                break;

            case GLFW_KEY_X:
                switch_terminal_ptr(st, st->messages);
                break;

        }    
    }

    terminal_handle_key_event(st, st->terminal);
    
    for(size_t i = 0; i < st->num_scripts; i++) {
        struct perl_script* script = &st->scripts[i];
        if(!script->is_loaded) {
            continue;
        }

        handle_script_keybind_event(st, script);
    }
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
    //struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);

    (void)window;
    (void)x_offset;
    (void)y_offset;

    // Currently not used...
}

void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);

    st->lfctx->win_width = width;
    st->lfctx->win_height = height;

    glViewport(0, 0, width, height);

    st->win_cols = width / st->font.char_width;
    st->win_rows = height / (st->font.char_height + st->cfg.main.line_padding);

    st->win_cols -= 1;
    st->win_rows -= 1;

    for(size_t i = 0; i < st->num_terminals; i++) {
        terminal_handle_resize_event(st, &st->terminals[i]);
    }

    leaf_free_framebuffer(&st->term_cells_framebuffer);
    leaf_free_framebuffer(&st->altrender_framebuffer);
    leaf_create_framebuffer(&st->term_cells_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    leaf_create_framebuffer(&st->altrender_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    //clear_region(st, 0, 0, st->lfctx->win_width, st->lfctx->win_height);
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
    return col * st->font.char_width + st->cfg.main.padding_x;
}

int rowtoy(struct nemi* st, int row) {
    return row * (st->font.char_height + st->cfg.main.line_padding) + st->cfg.main.padding_y;
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

void create_msg(struct nemi* st, const char* msg, ...) {
    va_list args;
    va_start(args, msg);

    char buffer[1024 * 4] = { 0 };
    vsnprintf(buffer, sizeof(buffer)-1, msg, args);

    write_term(st->messages, TERM_WRITE_VTERM, buffer);
    write_term(st->messages, TERM_WRITE_VTERM, "\n\r");
    switch_terminal_ptr(st, st->messages);

    logprintf(LOG_INFO, buffer);
    va_end(args);
}



void nemi_help(struct nemi* st, const char* what) {

    // Check if the string is equal to any script names
    // the user may be asking information about script.
    
    for(size_t i = 0; i < st->num_scripts; i++) {
        struct perl_script* script = &st->scripts[i];
        if(!script->is_loaded) {
            continue;
        }

        if(STR_MATCH(what, script->name)) {
            if(script->reg_events & REG_EVENT_HELP_MSG) {
                plscript_call(script, "event_help_message");
                return;
            }
            else {
                create_msg(st, "Help input \"%s\" matched with script name but it doesnt have help message available.", what);
            }
            
            //return;
        }
    }


    create_msg(st, "Sorry, didnt know how to help. Input: \"%s\"", what);
}


void nemi_message_script_keybinds(struct nemi* st, const char* script_name) {
    struct perl_script* script = NULL;
    for(size_t i = 0; i < st->num_scripts; i++) {
        if(st->scripts[i].is_loaded) {
            if(STR_MATCH(script_name, st->scripts[i].name)) {
                script = &st->scripts[i];
                break;
            }
        }
    }

    if(script == NULL) {
        create_msg(st, "\033[31mNo script named \"%s\"\033[0m", script_name);
        return;
    }

    if(!(script->reg_events & REG_EVENT_KEYBIND_PRESS)) {
        create_msg(st, "\033[33mScript \"%s\" has not registered keybind press event.\033[0m",
                script_name);
        return;
    }

    ptrdiff_t keybind_map_len = hmlen(script->keybind_map);
    if(keybind_map_len == 0) {
        create_msg(st, "\033[33mScript \"%s\" doesnt have any keybinds.\033[0m", script_name);
        return;
    }

    create_msg(st, "\033[92m%s \033[0m\033[32mkeybinds:\033[0m", script_name);
    

    struct string_t tmpkey_str = string_create(0);
    for(ptrdiff_t i = 0; i < keybind_map_len; i++) {
        struct script_keybind* kb = &script->keybind_map[i];
    
        create_msg(st, " %-26s %s", kb->value->event_name, kb->value->keys_str);
    }

    free_string(&tmpkey_str);
}

/*
void nemi_recompile_src(struct nemi* st) {
    if(st->cfg.main.source_dir == NULL
    || (strlen(st->cfg.main.source_dir) == 0)) {
        create_msg(st, "\033[31mRecompiling is disabled from configuration file.\033[0m");
        return;
    }

    if(!(st->flags & FLG_RECOMPILING_SUPPORTED)) {
        create_msg(st, "\033[31mRecompiling is not supported by current loader\n\r"
                       "or 'FLG_RECOMPILING_SUPPORTED' was not set by the loader.\033[0m");
        return;
    }

    st->flags |= FLG_LOADER_SHOULD_RECOMPILE;
}
*/

void restart_session(struct nemi* st) {
    if(!(st->flags & FLG_RESTARTING_SUPPORTED)) {
        create_msg(st, "\033[31mRestarting is not supported by current loader.\n\r"
                       "or 'FLG_RESTARTING_SUPPORTED' was not set by loader.\033[0m\n\r");
        return;
    }

    st->flags |= FLG_LOADER_RESTART_SESSION;
}

void hotreload_session(struct nemi* st) {
    if(!(st->flags & FLG_HOTRELOADING_SUPPORTED)) {
        create_msg(st, "\033[31mHotReloading is not supported by current loader.\n\r"
                       "or 'FLG_RESTARTING_SUPPORTED' was not set by loader.\033[0m\n\r");
        return;
    }

    st->flags |= FLG_LOADER_HOTRELOAD_SESSION;
}

