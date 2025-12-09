#include <stdlib.h>
#include <stdio.h>

#include "nemi.h"



void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    if(action != GLFW_PRESS) {
        return;
    }
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
    push_key_input(st, key);
    st->last_key_in = key;
}

void glfw_char_callback(GLFWwindow* window, uint32_t codepoint) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);
    if(codepoint >= 0x20 && codepoint <= 0x7E) {
        push_char_input(st, codepoint);
        st->last_char_in = codepoint;
    }
}

void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    struct nemi* st = (struct nemi*)glfwGetWindowUserPointer(window);

    st->lfctx->win_width = width;
    st->lfctx->win_height = height;

    glViewport(0, 0, width, height);
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

struct nemi* start_session() {
    struct nemi* st = malloc(sizeof *st);

    const char* font_file = "Topaz-8.ttf";

    st->flags = 0;
    st->lfctx = leaf_open("Nemi - Terminal Emulator", 800, 600);
    st->num_terminals = 0;

    zero_input_buffers(st);
    init_default_palette(st);

    if(!leaf_load_font(&st->font, "Topaz-8.ttf")) {
        quit_session(st);
        return NULL;
    }

    st->font.center_char_to_cell = true;
    st->font.spacing = 0.2f;
    leaf_set_font_scale(&st->font, 0.9);
    leaf_set_font_color(&st->font, 1.0, 0.8, 0.6);
 
    st->flags |= FLG_FONT_LOADED;

    st->terminal = spawn_terminal(st);

    glfwSetWindowUserPointer(st->lfctx->glfw_win, st);
    glfwSetKeyCallback        (st->lfctx->glfw_win, glfw_key_callback);
    glfwSetCharCallback       (st->lfctx->glfw_win, glfw_char_callback);
    glfwSetWindowSizeCallback (st->lfctx->glfw_win, glfw_window_resize_callback);

    const size_t renderer_memory_size = 1024 * sizeof(float);
    leaf_init_renderer(st->lfctx, renderer_memory_size);

    st->line_padding_y = 2;

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
}

void zero_input_buffers(struct nemi* st) {
    for(int i = 0; i < NEMI_KEYINBUF_MAX; i++) {
        st->key_inputs[i] = 0;
    }
    for(int i = 0; i < NEMI_CHARINBUF_MAX; i++) {
        st->char_inputs[i] = 0;
    }
}

void end_frame(struct nemi* st) {
    st->last_char_in = 0;
    st->last_key_in = 0;
    glfwSwapBuffers(st->lfctx->glfw_win);
    glfwPollEvents();
}

void set_palette_color(struct nemi* st, int color_id, struct rgb_color color) {
    st->palette[CHAR_COLOR__END - color_id] = color;
}

struct rgb_color get_palette_color(struct nemi* st, int color_id) {
    return st->palette[CHAR_COLOR__END - color_id];
}

void init_default_palette(struct nemi* st) {
    set_palette_color(st, CHAR_COLOR_DEFAULT,        (struct rgb_color){ 180, 160, 150 });
    set_palette_color(st, CHAR_COLOR_BLACK,          (struct rgb_color){ 30,  30,  30 });
    set_palette_color(st, CHAR_COLOR_RED,            (struct rgb_color){ 180, 30,  30 });
    set_palette_color(st, CHAR_COLOR_GREEN,          (struct rgb_color){ 30,  180, 30 });
    set_palette_color(st, CHAR_COLOR_YELLOW,         (struct rgb_color){ 180, 180, 30 });
    set_palette_color(st, CHAR_COLOR_BLUE,           (struct rgb_color){ 30,  30,  180 });
    set_palette_color(st, CHAR_COLOR_MAGENTA,        (struct rgb_color){ 180, 30,  180 });
    set_palette_color(st, CHAR_COLOR_CYAN,           (struct rgb_color){ 30,  180, 180 });
    set_palette_color(st, CHAR_COLOR_WHITE,          (struct rgb_color){ 180, 180, 180 });
    
    set_palette_color(st, CHAR_COLOR_BRIGHT_BLACK,   (struct rgb_color){ 30,  30,  30 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_RED,     (struct rgb_color){ 255, 30,  30 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_GREEN,   (struct rgb_color){ 30,  255, 30 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_YELLOW,  (struct rgb_color){ 255, 255, 30 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_BLUE,    (struct rgb_color){ 30,  30,  255 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_MAGENTA, (struct rgb_color){ 255, 30,  255 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_CYAN,    (struct rgb_color){ 30,  255, 255 });
    set_palette_color(st, CHAR_COLOR_BRIGHT_WHITE,   (struct rgb_color){ 255, 255, 255 });
}

void to_grid_pos(struct nemi* st, int* x, int* y) {
    *x *= st->font.char_width;
    *y *= st->font.char_height;
    *x += 10;
    *y += 10;
}

