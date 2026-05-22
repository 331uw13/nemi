
#include "glfw_callbacks.h"

#include "nemi.h"


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; 
    
    if(action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    
    Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);
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

    /*
    if(mods == GLFW_MOD_CONTROL) {
        switch(key) {
       
            case GLFW_KEY_SPACE:
                nmt_switch_terminal_ptr(st, st->terminal_prev);
                break;

            case GLFW_KEY_X:
                nmt_switch_terminal_ptr(st, st->messages);
                break;

                //...
        }    
    }
    */

}

void glfw_char_callback(GLFWwindow* window, uint32_t codepoint) {
    Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);
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

void glfw_scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    //Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);

    (void)window;
    (void)x_offset;
    (void)y_offset;

    // Currently not used...
}

void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    Nemi* st = (Nemi*)glfwGetWindowUserPointer(window);

    st->lfctx->win_width = width;
    st->lfctx->win_height = height;

    glViewport(0, 0, width, height);

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

