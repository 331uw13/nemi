#include <stdio.h>
#include <string.h>

#include "log.h"
#include "nemi.h"



static const int keybind_toggle
[] = {
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_0
};

static bool active = false;


static struct {
    int cmd_cursor;
    int cmd_box_x;
    int cmd_box_y;
    struct string_t cmd_str;
}
global = {
    .cmd_cursor = 0,
    .cmd_box_x = 5,
    .cmd_box_y = 5
};


void enable_module(bool enabled) { 
    Nemi* st = nmt_getst();
    if(enabled) {
        st->term_ignore_char_input_counter++;
        st->term_ignore_key_input_counter++;

        st->term_cells_render_offset_y = st->font.char_height;

        string_clear(&global.cmd_str);
        global.cmd_cursor = 0;
    }
    else {
        st->term_cells_render_offset_y = 0;
        st->term_ignore_char_input_counter--;
        st->term_ignore_key_input_counter--;
    }

    active = enabled;
}

void keybind_toggle_fn() {
    Nemi* st = nmt_getst();

    active = !active;
    enable_module(active);
}


void process_cmd_str() {
    Nemi* st = nmt_getst();

    if(strcmp(global.cmd_str.bytes, "msg") == 0) {
        nmt_switch_terminal_ptr(st, st->messages);
    }
    else
    if(strcmp(global.cmd_str.bytes, "pt") == 0) {
        nmt_switch_terminal_ptr(st, st->terminal_prev);
    }

}

// Event
void module_event_char_input(char c) {
    //string_append(&global.cmd_str, &c, 1);
    string_addbyte(&global.cmd_str, c, global.cmd_cursor);
    global.cmd_cursor++;
}

// Event
void module_event_key_input(int key, int modifiers) {
    if(!active) {
        return;
    }

    switch(key) {
        case GLFW_KEY_ENTER:
            string_nullterm(&global.cmd_str);
            process_cmd_str();
            enable_module(false);
            break;
    
        case GLFW_KEY_LEFT:
            if(global.cmd_cursor > 0) {
                global.cmd_cursor--;
            }
            break;
    
        case GLFW_KEY_RIGHT:
            if(global.cmd_cursor+1 <= global.cmd_str.size) {
                global.cmd_cursor++;
            }
            break;


        case GLFW_KEY_BACKSPACE:
            if(global.cmd_cursor > 0) {
                string_delbyte(&global.cmd_str, global.cmd_cursor);
                global.cmd_cursor--;
            }
            break;
    } 
}

// Event
void module_event_render() {
    if(!active) {
        return;
    }

    Nemi* st = nmt_getst();


    int box_width = st->lfctx->win_width;
    leaf_draw_rect(
            global.cmd_box_x,
            global.cmd_box_y,
            box_width,
            st->font.char_height, 
            (struct color_t) {
                6, 6, 6
            });


    st->font.char_color_r = 0.2f;
    st->font.char_color_g = 0.9f;
    st->font.char_color_b = 0.9f;
    
    leaf_draw_char(&st->font, 
            global.cmd_box_x,
            global.cmd_box_y,
            ';');

    leaf_draw_text(&st->font, 
            global.cmd_box_x + st->font.char_width+10,
            global.cmd_box_y,
            global.cmd_str.bytes, global.cmd_str.size);

    leaf_draw_rect(
            global.cmd_box_x + st->font.char_width+10 + global.cmd_cursor * st->font.char_width,
            global.cmd_box_y + st->font.char_height,
            st->font.char_width,
            2,
            (struct color_t) {
                0, 255, 255
            });
}

void module_loaded(size_t module_idx) {

    logprintf(LOG_INFO, "Test module was loaded!");


    Nemi* nemi = nmt_getst();

    global.cmd_str = string_create(0);

    nmt_assign_module_keybind(nemi, module_idx,
            keybind_toggle_fn, keybind_toggle, ARRAY_LEN(keybind_toggle));




    logprintf(LOG_INFO, "State pointer from module = %p", nemi);
}


void module_quit() {
    free_string(&global.cmd_str);
    logprintf(LOG_INFO, "Test module was closed!");
}


