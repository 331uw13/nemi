/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/
#include "userinput_callbacks.h"

#include "nemi.h"


void userinput_mouse_moved(void* user_pointer, float new_x, float new_y) {
    Nemi* st = (Nemi*)user_pointer;
    st->mouse_x = new_x;
    st->mouse_y = new_y;

    for(size_t i = 0; i < st->num_loaded_modules; i++) {
        NModule* module = &st->modules[i];

        if(module->events.fn_mouse_moved) {
            module->events.fn_mouse_moved(new_x, new_y);
        }
    }
}

void userinput_mouse_pressed(void* user_pointer, int button) {
    Nemi* st = (Nemi*)user_pointer;
    st->last_mouse_button = button;

    for(size_t i = 0; i < st->num_loaded_modules; i++) {
        NModule* module = &st->modules[i];

        if(module->events.fn_mouse_pressed) {
            module->events.fn_mouse_pressed(button);
        }
    }
}

void userinput_mouse_scroll(void* user_pointer, int direction) {
    Nemi* st = (Nemi*)user_pointer;
    
    for(size_t i = 0; i < st->num_loaded_modules; i++) {
        NModule* module = &st->modules[i];

        if(module->events.fn_mouse_scroll) {
            module->events.fn_mouse_scroll(direction);
        }
    }
}

void userinput_key_pressed(void* user_pointer, int key, int mods) {
    Nemi* st = (Nemi*)user_pointer;
    nmt_push_key_input(st, key);

    st->last_key_in = key;
    st->last_keymod_in = mods;
    
    nmt_keyinput_events_for_modules(st, key, mods);
    
    if(st->inputfocus_module_idx >= 0) {
        NModule* module = &st->modules[st->inputfocus_module_idx];
        if(module->events.fn_key_input) {
            module->events.fn_key_input(key, mods);
        }
        return;
    }

    nmterm_handle_key_event(st, st->terminal);


    for(size_t i = 0; i < st->num_loaded_modules; i++) {
        NModule* module = &st->modules[i];

        if(module->events.fn_key_input) {
            module->events.fn_key_input(key, mods);
        }
    }
}

void userinput_char_pressed(void* user_pointer, uint32_t codepoint) {
    Nemi* st = (Nemi*)user_pointer;
    if(codepoint >= 0x20 && codepoint <= 0x7E) {

        nmt_push_char_input(st, codepoint);
        st->last_char_in = codepoint;


        if(st->inputfocus_module_idx >= 0) {
            NModule* module = &st->modules[st->inputfocus_module_idx];
            if(module->events.fn_char_input) {
                module->events.fn_char_input((char)codepoint);
            }
            return;
        }
        

        nmterm_handle_char_event(st, st->terminal);
   
        for(size_t i = 0; i < st->num_loaded_modules; i++) {
            NModule* module = &st->modules[i];

            if(module->events.fn_char_input) {
                module->events.fn_char_input((char)codepoint);
            }
        }
    }
}


void userinput_window_resized(void* user_pointer, int width, int height) {
    Nemi* st = (Nemi*)user_pointer;

    st->lfctx->win_width = width;
    st->lfctx->win_height = height;

    leaf_set_viewport(0, 0, width, height);

    st->win_cols = width / st->font.char_width;
    st->win_rows = height / (st->font.char_height + st->cfg.main.line_padding);

    st->win_cols -= 1;
    st->win_rows -= 1;
    

    for(size_t i = 0; i < st->num_terminals; i++) {
        nmterm_handle_resize_event(st, &st->terminals[i]);
    }

    leaf_free_framebuffer(&st->term_cells_framebuffer);
    leaf_free_framebuffer(&st->altrender_framebuffer);
    leaf_create_framebuffer(&st->term_cells_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    leaf_create_framebuffer(&st->altrender_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    //clear_region(st, 0, 0, st->lfctx->win_width, st->lfctx->win_height);

    // Inform modules.

    for(size_t i = 0; i < st->num_loaded_modules; i++) {
        NModule* module = &st->modules[i];

        if(module->events.fn_window_resized) {
            module->events.fn_window_resized();
        }
    }
}


/*
void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);

    st->mouse_x = (float)xpos;
    st->mouse_y = (float)ypos;

}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);
    (void)mods;

    if(action == GLFW_PRESS) {
        st->last_mouse_button = button;
    }
}
*/
