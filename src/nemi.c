#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

#include "nemi.h"
#include "nemi_config.h"
#include "memory.h"
#include "fileio.h"
#include "glfw_callbacks.h"

#include "thirdparty/stb_ds.h"



static Nemi* g_nemi_state = NULL;



Nemi* nmt_getst() {
    return g_nemi_state;
}

void nmt_prepare_from_hotreload(Nemi* st) {
    leaf_set_drawing_context(st->lfctx);
    g_nemi_state = st;
}

Nemi* nmt_start_session(NemiFilepaths filepaths) {

    setlocale(LC_ALL, "C"); // Restart from loader may mess with locale.
    Nemi* st = malloc(sizeof *st);

    st->filepaths = filepaths; 
    st->frame_time = 0.001;
    st->frame_time_begin = 0.0;
    st->flags = 0;
    st->lfctx = leaf_open("Nemi - Terminal Emulator", 900, 700, 0);
    st->num_terminals = 0;
    st->inputfocus_module_idx = -1;
    st->term_cells_render_offset_x = 0;
    st->term_cells_render_offset_y = 0;
    nmt_zero_input_buffers(st);
    nmt_init_default_config(st);

    if(!nmt_read_configs(st, st->filepaths.configs)) {
        logprintf(LOG_ERROR, "Failed to read configs from '%s'", st->filepaths.configs);
        nmt_quit_session(st);
        return NULL;
    }



    st->font.loaded = false;

    if(access(st->cfg.font.filepath, R_OK) == 0) {
        leaf_load_font(&st->font, st->cfg.font.filepath);
    }
    else {
        // Font was not found, Lets try from 'filepaths.fonts + font.filepath' next. 
        struct string_t font_path = string_create(0);
        string_append(&font_path, filepaths.fonts, strlen(filepaths.fonts));
        if(string_lastbyte(&font_path) != '/' && st->cfg.font.filepath[0] != '/') {
            string_pushbyte(&font_path, '/');
        }
        string_append(&font_path, st->cfg.font.filepath, strlen(st->cfg.font.filepath));

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
        nmt_quit_session(st);
        return NULL;
    }
   
    

    st->font.center_char_to_cell = st->cfg.font.center_char_to_cell;
    st->font.spacing = 0.2f;
    leaf_set_font_space_width(&st->font, st->font.max_bitmap_width / 2.0f);
    leaf_set_font_scale(&st->font, st->cfg.font.scale);

    leaf_set_font_color(&st->font, (RGBColor) { 255, 200, 150 });


    glfwSetWindowUserPointer(st->lfctx->glfw_win, st);
    glfwSetInputMode          (st->lfctx->glfw_win, GLFW_STICKY_KEYS, GLFW_FALSE);
    glfwSetKeyCallback        (st->lfctx->glfw_win, glfw_key_callback);
    glfwSetCharCallback       (st->lfctx->glfw_win, glfw_char_callback);
    glfwSetScrollCallback     (st->lfctx->glfw_win, glfw_scroll_callback);
    glfwSetWindowSizeCallback (st->lfctx->glfw_win, glfw_window_resize_callback);
    glfwSetCursorPosCallback  (st->lfctx->glfw_win, glfw_cursor_position_callback);
    glfwSetMouseButtonCallback(st->lfctx->glfw_win, glfw_mouse_button_callback);

    // This will allocate memory for the renderer's
    // vertex buffer object. Only one is used. 
    const size_t renderer_memory_size = 1024 * sizeof(float);
    leaf_init_renderer(st->lfctx, renderer_memory_size);
    

    st->win_cols = st->lfctx->win_width / st->font.char_width;
    st->win_rows = st->lfctx->win_height / (st->font.char_height + st->cfg.main.line_padding);
    st->win_cols -= 1;
    st->win_rows -= 1;

    // Spawn default terminals.
    st->messages = nmterm_spawn(st, st->win_rows, st->win_cols, ECHO_TERMINAL);
    st->terminal = nmterm_spawn(st, st->win_rows, st->win_cols, SHELL_TERMINAL);
    st->terminal_prev = st->terminal;
   

    nmterm_write(st->messages, TERM_WRITE_VTERM, 
            "\033[2J\033[H\033[0m\033[90m(start of messages)\033[0m\n\r");

  

    g_nemi_state = st;

    logprintf(LOG_INFO, "Global state pointer = %p", g_nemi_state);

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
    
    st->num_loaded_modules = 0;
    st->modules = calloc(NEMI_MODULES_MAX, sizeof *st->modules);
    nmt_load_all_modules(st);
    

    logprintf(LOG_INFO, "Session started, Version " NEMI_VERSION_STR);
    return st;
}

void nmt_quit_session(Nemi* st) {
    leaf_unload_font(&st->font);

    for(uint16_t i = 0; i < st->num_terminals; i++) {
        nmterm_close(&st->terminals[i]);
    }

    glfwSetWindowUserPointer  (st->lfctx->glfw_win, NULL);
    glfwSetKeyCallback        (st->lfctx->glfw_win, NULL);
    glfwSetCharCallback       (st->lfctx->glfw_win, NULL);
    glfwSetScrollCallback     (st->lfctx->glfw_win, NULL);
    glfwSetWindowSizeCallback (st->lfctx->glfw_win, NULL);

    leaf_free_renderer(st->lfctx);
    leaf_quit(st->lfctx);

    for(size_t i = 0; i < NEMI_MODULES_MAX; i++) {
        nmt_module_quit(&st->modules[i]);
    }

    leaf_free_framebuffer(&st->term_cells_framebuffer);
    nmt_free_configs(st);

    log_close();
    freeif(st);
}

void nmt_load_all_modules(Nemi* st) {
    
    DIR* dir = opendir(st->filepaths.modules);
    if(!dir) {
        logprintf(LOG_ERROR, "Failed to open modules directory | %s", 
                strerror(errno));
        return;
    }

    char elf_magic_bytes[4] = {
        0x7F,
        0x45,
        0x4C,
        0x46
    };


    struct string_t path_str = string_create(0);
    struct dirent* ent = NULL;

    NModule* module_ptr = &st->modules[0];

    while((ent = readdir(dir)) != NULL) {

        if(ent->d_name[0] == '.') {
            continue;
        }

        string_clear(&path_str);
        string_append(&path_str, st->filepaths.modules, strlen(st->filepaths.modules));
        if(string_lastbyte(&path_str) != '/') {
            string_pushbyte(&path_str, '/');
        }
        string_append(&path_str, ent->d_name, strlen(ent->d_name));
        string_nullterm(&path_str);


        char* magic_bytes = file_magic_bytes(path_str.bytes, 4*sizeof(char));
        if(!magic_bytes) {
            continue;
        }

        if(memcmp(magic_bytes, elf_magic_bytes, 4) != 0) {
            continue;
        }


        // This file should be confirmed now.
        
        if(nmt_module_load(module_ptr, path_str.bytes)) {
            // Inform the module it was loaded.
            // By passing its index it can assign keybinds and in future maybe other things.
            size_t loaded_module_index = st->num_loaded_modules;
            

            logprintf(LOG_INFO, "Calling '%s' module_loaded(module_idx=%li)", module_ptr->path, loaded_module_index);
            module_ptr->fn_loaded(loaded_module_index);
            
            st->num_loaded_modules++;
            module_ptr++;
        }

        if(st->num_loaded_modules+1 >= NEMI_MODULES_MAX) {
            logprintf(LOG_WARN, "Reached max amount of loaded modules.");
            break;
        }
    }

    free_string(&path_str);
    closedir(dir);
}



void nmt_zero_input_buffers(Nemi* st) {
    for(int i = 0; i < NEMI_KEYINBUF_MAX; i++) {
        st->key_inputs[i] = 0;
    }
    for(int i = 0; i < NEMI_CHARINBUF_MAX; i++) {
        st->char_inputs[i] = 0;
    }
}

void nmt_switch_terminal_idx(Nemi* st, uint32_t index) {
    nmt_switch_terminal_ptr(st, &st->terminals[index]);
}

void nmt_switch_terminal_ptr(Nemi* st, NTerminal* term) {

    logprintf(LOG_INFO, "Switching terminals: (from)%p -> (to)%p",
            st->terminal,
            term);

    if(st->terminal == term) {
        return;
    }
    st->terminal_prev = st->terminal;
    st->terminal = term;

    for(int row = 0; row < term->rows; row++) {
        term->dirty_rows[row] = true;
    }
}



#define CLEAR_COLOR_NO_ALPHA 0.0f
#define CLEAR_COLOR_INCLUDE_ALPHA 1.0f
void nmt_clear_color_buffer_bit(Nemi* st, float clear_color_alpha) {
    glClearColor(
            (float)st->cfg.colors[NEMI_COLOR_BG].r / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].g / 255.0f,
            (float)st->cfg.colors[NEMI_COLOR_BG].b / 255.0f,
            clear_color_alpha);
    glClear(GL_COLOR_BUFFER_BIT);
}

void nmt_clear_region(Nemi* st, int x, int y, int w, int h) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, (st->lfctx->win_height - h) - y, w, h);
    nmt_clear_color_buffer_bit(st, CLEAR_COLOR_NO_ALPHA);
    glDisable(GL_SCISSOR_TEST);
}





static
void p_nmterm_select_callback__render(void* user_pointer, ssize_t row, ssize_t row_length, ssize_t col_begin) {
    Nemi* st = (Nemi*)user_pointer;
    leaf_draw_rect
    (
        nmt_coltox(st, col_begin),
        nmt_rowtoy(st, row - st->terminal->yscroll),
        st->font.char_width * row_length,
        st->font.char_height + st->cfg.main.line_padding,
        st->cfg.colors[NEMI_COLOR_TERM_SELECT_REG]   
    );
}

static
size_t p_nmterm_select_callback__get_row_length(void* user_pointer, ssize_t row) {
    Nemi* st = (Nemi*)user_pointer;
    return nmterm_get_row_length(st->terminal, row);
}

void nmt_update_frame(Nemi* st) {
    st->frame_time_begin = glfwGetTime();



    nmterm_update_blink_timer(st, st->terminal);

    // Render terminal cells to 'term_cells_framebuffer'
    leaf_use_framebuffer(&st->term_cells_framebuffer);
    {
        // We are not going to clear the color buffer bit here
        // for the whole framebuffer before rendering, because
        // the terminal's keep track of what is needed to be rendered again.
        // And saves the result to the framebuffer's texture so it can be reused.
        nmterm_read(st, st->terminal);
        nmterm_render(st, st->terminal);


        leaf_font_render(&st->font);
        leaf_renderer_flush(st->lfctx);
    }

    // Render anything else to 'altrender_framebuffer'
    leaf_use_framebuffer(&st->altrender_framebuffer);
    {
        // For alternative framebuffer the whole screen is cleared
        // before rendering again, because the modules can render
        // to arbitrary coordinates and not just cell coordinates
        // so it is more harder to keep track of what has changed.
        // So... it will be bit slower at least for now.
        nmt_clear_color_buffer_bit(st, CLEAR_COLOR_NO_ALPHA);
 
        // The select region is needed to render separatly from cells.
        // because the terminal will only render when cells change and we are not changing cells data.
        if(st->terminal->select.active) {
            nmt_select_process
            (
                st->terminal->select,
                p_nmterm_select_callback__get_row_length,
                p_nmterm_select_callback__render,
                st
            );
            //nmterm_process_select_region(st, st->terminal, p_nmt_term_select_process_callback__render);
        }       

        for(size_t i = 0; i < st->num_loaded_modules; i++) {
            NModule* module = &st->modules[i];
            if(module->events.fn_render) {
                module->events.fn_render();
            }
        }

        leaf_font_render(&st->font);
        leaf_renderer_flush(st->lfctx);
    }


    leaf_use_framebuffer(NULL);
    nmt_clear_color_buffer_bit(st, CLEAR_COLOR_INCLUDE_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);

    leaf_draw_texture_rect(0, 0,
            st->altrender_framebuffer.width,
            st->altrender_framebuffer.height,
            st->altrender_framebuffer.texture,
            (RGBColor){ 255, 255, 255 },
            LEAF_TEXTURE_NO_OPTIONS);

    leaf_draw_texture_rect(
             st->term_cells_render_offset_x,
            -st->term_cells_render_offset_y,
            st->term_cells_framebuffer.width,
            st->term_cells_framebuffer.height,
            st->term_cells_framebuffer.texture,
            (RGBColor){ 255, 255, 255 },
            LEAF_TEXTURE_NO_OPTIONS);


    //leaf_renderer_flush(st->lfctx);
    //leaf_font_render(&st->font);

    st->last_char_in = 0;
    st->last_key_in = 0;
    st->last_mouse_button = -1;
    glfwSwapBuffers(st->lfctx->glfw_win);
    
    glfwPollEvents();
    //usleep(10000);

    st->frame_time = glfwGetTime() - st->frame_time_begin;

}

bool nmt_key_down(Nemi* st, int key) {
    return (glfwGetKey(st->lfctx->glfw_win, key) == GLFW_PRESS);
}

void nmt_init_default_config(Nemi* st) {
    st->cfg.main.padding_x = 10;
    st->cfg.main.padding_y = 10;
    st->cfg.main.line_padding = 3;
    
    int d_v = 180;  // Dim max.
    int b_v = 255;  // Bright max.
    int l_v = 30;   // Low value color component.

    st->cfg.colors[NEMI_COLOR_FG] = (RGBColor){ 200, 180, 160 };
    st->cfg.colors[NEMI_COLOR_BG] = (RGBColor){ 4, 4, 10 };
    st->cfg.colors[NEMI_COLOR_BLACK]   = (RGBColor){ 0, 0, 0 };
    st->cfg.colors[NEMI_COLOR_RED]     = (RGBColor){ d_v, l_v, l_v };
    st->cfg.colors[NEMI_COLOR_GREEN]   = (RGBColor){ l_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_YELLOW]  = (RGBColor){ d_v, d_v, l_v };
    st->cfg.colors[NEMI_COLOR_BLUE]    = (RGBColor){ l_v + 40, l_v + 40, d_v };
    st->cfg.colors[NEMI_COLOR_MAGENTA] = (RGBColor){ d_v, l_v, d_v };
    st->cfg.colors[NEMI_COLOR_CYAN]    = (RGBColor){ l_v, d_v, d_v };
    st->cfg.colors[NEMI_COLOR_WHITE]   = (RGBColor){ d_v, d_v, d_v };

    st->cfg.colors[NEMI_BRIGHT_COLOR_BLACK]   = (RGBColor){ 70, 70, 70 };
    st->cfg.colors[NEMI_BRIGHT_COLOR_RED]     = (RGBColor){ b_v, l_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_GREEN]   = (RGBColor){ l_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_YELLOW]  = (RGBColor){ b_v, b_v, l_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_BLUE]    = (RGBColor){ l_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_MAGENTA] = (RGBColor){ b_v, l_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_CYAN]    = (RGBColor){ l_v, b_v, b_v };
    st->cfg.colors[NEMI_BRIGHT_COLOR_WHITE]   = (RGBColor){ b_v, b_v, b_v };

    st->cfg.colors[NEMI_COLOR_MESSAGES_FG] = (RGBColor){ 200, 120, 60 };
    st->cfg.colors[NEMI_COLOR_MESSAGES_BG] = (RGBColor){ 20, 10, 6 };
}


size_t nmt_hash_glfwkeys(const int* keys, size_t num_keys) {
    size_t hash = 15;
    for(size_t i = 0; i < num_keys; i++) {
        hash = hash * 31 ^ keys[i];
    }
    return hash;
}


static
int p_nmt_qsort_glfwkeys_callback(const void* a, const void* b) {
    int i_a = *(int*)a;
    int i_b = *(int*)b;
    return i_a >= i_b;
}

void nmt_assign_module_keybind(Nemi* st, size_t module_idx, void(*fnptr)(), const int* keys, size_t num_keys) {
    NModule* module = &st->modules[module_idx];


    if(module->num_keybinds+1 >= MODULE_KEYBINDS_MAX) {
        logprintf(LOG_ERROR, "Module has max amount of keybinds.");
        return;
    }


    NModuleKeybind* keybind = &module->keybinds[module->num_keybinds];


    int sorted_keys[num_keys];
    memcpy(sorted_keys, keys, num_keys * sizeof *keys);

    qsort(sorted_keys, num_keys, sizeof(int), p_nmt_qsort_glfwkeys_callback);


    keybind->key_hash = nmt_hash_glfwkeys(sorted_keys, num_keys);
    keybind->fn_ptr = fnptr;

    module->num_keybinds++;
}


static
void p_nmt_call_matching_module_keybind_funcs(NModule* module, size_t keys_hash) {
    for(size_t k = 0; k < module->num_keybinds; k++) {
        NModuleKeybind* keybind = &module->keybinds[k];
        if(keybind->fn_ptr == NULL) {
            continue;
        }

        if(keybind->key_hash == keys_hash) {
            keybind->fn_ptr();
        }
    }
}

void nmt_keyinput_events_for_modules(Nemi* st, int key, int mods) {
    if(st->num_loaded_modules == 0) {
        return;
    }

    int pressed_keys[GLFW_KEY_LAST] = { 0 };
    size_t num_pressed_keys = 0;

    for(int i = GLFW_KEY_SPACE; i < GLFW_KEY_LAST; i++) {
        if(glfwGetKey(st->lfctx->glfw_win, i) == GLFW_PRESS) {
            
            // There is maybe some issue how glfw resets some key states when glfwGetKey is used...
            // At least on i3wm, the super key state may never be reset until its pressed again.
            // That will result the keyhash being "wrong"
            
            // Maybe there is some other way to fix this but for now
            // lets double check the key modifiers.

            if(i == GLFW_KEY_LEFT_ALT || i == GLFW_KEY_RIGHT_ALT) {
                if(!(mods & GLFW_MOD_ALT)) {
                    continue;
                }
            }
            else
            if(i == GLFW_KEY_LEFT_SUPER || i == GLFW_KEY_RIGHT_SUPER) {
                if(!(mods & GLFW_MOD_SUPER)) {
                    continue;
                }
            }
            else
            if(i == GLFW_KEY_LEFT_CONTROL || i == GLFW_KEY_RIGHT_CONTROL) {
                if(!(mods & GLFW_MOD_CONTROL)) {
                    continue;
                }
            }
            else
            if(i == GLFW_KEY_LEFT_SHIFT || i == GLFW_KEY_RIGHT_SHIFT) {
                if(!(mods & GLFW_MOD_SHIFT)) {
                    continue;
                }
            }
            
            pressed_keys[num_pressed_keys] = i;
            num_pressed_keys++;
        }
    }


    size_t pressed_keys_hash = nmt_hash_glfwkeys(pressed_keys, num_pressed_keys);

    if(st->inputfocus_module_idx >= 0) {
        NModule* module = &st->modules[st->inputfocus_module_idx];
        p_nmt_call_matching_module_keybind_funcs(module, pressed_keys_hash);
    }
    else {
        for(size_t i = 0; i < st->num_loaded_modules; i++) {
            NModule* module = &st->modules[i];
            p_nmt_call_matching_module_keybind_funcs(module, pressed_keys_hash);
        }
    }
}

void nmt_push_key_input(Nemi* st, int key) {
    for(int i = NEMI_KEYINBUF_MAX-1; i > 0; i--) {
        st->key_inputs[i] = st->key_inputs[i-1];
    }
    st->key_inputs[0] = key;
}

void nmt_push_char_input(Nemi* st, char ch) {
    for(int i = NEMI_CHARINBUF_MAX-1; i > 0; i--) {
        st->char_inputs[i] = st->char_inputs[i-1];
    }
    st->char_inputs[0] = ch;
}

int nmt_coltox(Nemi* st, int col) {
    return col * st->font.char_width + st->cfg.main.padding_x;
}

int nmt_rowtoy(Nemi* st, int row) {
    return row * (st->font.char_height + st->cfg.main.line_padding) + st->cfg.main.padding_y;
}

void nmt_font_scale(Nemi* st, float offset) {
    leaf_set_font_scale(&st->font, st->font.scale + offset);

    // We can call glfw window resize callback here
    // it resizes the terminals rows and columns too.
    glfw_window_resize_callback(st->lfctx->glfw_win, st->lfctx->win_width, st->lfctx->win_height);
}

void nmt_set_font_scale(Nemi* st, float scale) {
    leaf_set_font_scale(&st->font, scale);
    glfw_window_resize_callback(st->lfctx->glfw_win, st->lfctx->win_width, st->lfctx->win_height);
}

void nmt_create_msg(Nemi* st, const char* msg, ...) {
    va_list args;
    va_start(args, msg);

    char buffer[1024 * 4] = { 0 };
    vsnprintf(buffer, sizeof(buffer)-1, msg, args);

    nmterm_write(st->messages, TERM_WRITE_VTERM, buffer);
    nmterm_write(st->messages, TERM_WRITE_VTERM, "\n\r");
    nmt_switch_terminal_ptr(st, st->messages);


    logprintf(LOG_INFO, buffer);
    va_end(args);
}

bool nmt_is_module_inputfocus_available(Nemi* st) {
    return st->inputfocus_module_idx < 0;
}

bool nmt_module_gain_inputfocus(Nemi* st, size_t module_idx) {
    NModule* asking_module = &st->modules[module_idx];

    if(st->inputfocus_module_idx == module_idx) {
        logprintf(LOG_WARN, "Module '%s' already has module inputfocus",
                asking_module->path);
        return false;
    }

    if(!nmt_is_module_inputfocus_available(st)) {
        NModule* owner_module = &st->modules[st->inputfocus_module_idx];

        logprintf(LOG_ERROR, "'%s' Cannot gain module inputfocus, because its currently owned by '%s'",
                asking_module->path,
                owner_module->path
        );
        return false;
    }

    logprintf(LOG_INFO, "Module inputfocus transfered to '%s'", asking_module->path);
    st->inputfocus_module_idx = module_idx;
    return true;
}

// Only the module which has gained inputfocus can deactivate it.
void nmt_module_free_inputfocus(Nemi* st, size_t module_idx) {
    NModule* asking_module = &st->modules[module_idx];

    if(st->inputfocus_module_idx == (ssize_t)module_idx) {
        st->inputfocus_module_idx = -1;
        logprintf(LOG_INFO, "Module inputfocus freed by '%s'", asking_module->path);
    }
    else
    if(st->inputfocus_module_idx >= 0) {
        NModule* owner_module = &st->modules[st->inputfocus_module_idx];
    
        logprintf(LOG_ERROR, "'%s' Cannot free module inputfocus, because its currently owned by '%s'",
                asking_module->path,
                owner_module->path
        );
    }
}

/*
void nemi_help(Nemi* st, const char* what) {

    // Check if the string is equal to any script names
    // the user may be asking information about script.
    
    for(size_t i = 0; i < st->num_scripts; i++) {
        PerlScript* script = &st->scripts[i];
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
*/


/*
void nemi_message_script_keybinds(Nemi* st, const char* script_name) {
    PerlScript* script = NULL;
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
*/

/*
void nemi_recompile_src(Nemi* st) {
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

/*
void restart_session(Nemi* st) {
    if(!(st->flags & FLG_RESTARTING_SUPPORTED)) {
        create_msg(st, "\033[31mRestarting is not supported by current loader.\n\r"
                       "or 'FLG_RESTARTING_SUPPORTED' was not set by loader.\033[0m\n\r");
        return;
    }

    st->flags |= FLG_LOADER_RESTART_SESSION;
}

void hotreload_session(Nemi* st) {
    if(!(st->flags & FLG_HOTRELOADING_SUPPORTED)) {
        create_msg(st, "\033[31mHotReloading is not supported by current loader.\n\r"
                       "or 'FLG_RESTARTING_SUPPORTED' was not set by loader.\033[0m\n\r");
        return;
    }

    st->flags |= FLG_LOADER_HOTRELOAD_SESSION;
}

const char* nemi_get_clipboard_content(Nemi* st) {
    return glfwGetClipboardString(st->lfctx->glfw_win);
}
*/

