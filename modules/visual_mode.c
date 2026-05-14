#include "nemi.h"


static const int keybind_toggle[] = {
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_K
};




static struct {
    bool   enabled;
    size_t module_idx;
    int    vcursor_col;
    int    vcursor_row;
}
g = {
    .module_idx = 0
};



void module_event_render() {
    if(!g.enabled) {
        return;
    }

    Nemi* st = nmt_getst();

    if(st->terminal->select.active) {
        leaf_draw_rect(0, 0, 10, 10, (struct color_t){ 255, 70, 160 });
    }

    leaf_draw_rect(
            nmt_coltox(st, g.vcursor_col),
            nmt_rowtoy(st, g.vcursor_row - st->terminal->yscroll) + st->font.char_height,
            st->font.char_width,
            2,
            (struct color_t){ 10, 160, 10 });
}

void keybind_toggle_fn() {
    Nemi* st = nmt_getst();
    g.enabled = !g.enabled;
    
    if(g.enabled) {
        if(!nmt_module_gain_inputfocus(st, g.module_idx)) {
            g.enabled = false;
            return; // Failed to gain focus.
        }

        g.vcursor_col = st->terminal->cursor_col+1;
        g.vcursor_row = st->terminal->cursor_row-1;
    }
    else {
        nmt_module_free_inputfocus(st, g.module_idx);
    }
}

void module_event_key_input(int key, int modifiers) {
    if(!g.enabled) {
        return;
    }

    Nemi* st = nmt_getst();

    bool cursor_moved = false;
    switch(key) {

        case GLFW_KEY_S:
            if(st->terminal->select.active) {
                nmterm_select_end(st->terminal);
            }
            else {
                nmterm_select_begin(st->terminal, g.vcursor_col, g.vcursor_row);
            }
            break;

        case GLFW_KEY_I:
        case GLFW_KEY_UP:
            g.vcursor_row--;
            cursor_moved = true;
            if(modifiers & GLFW_MOD_SHIFT) {
                nmterm_yscroll(st->terminal, -1);
            }
            break;

        case GLFW_KEY_K:
        case GLFW_KEY_DOWN:
            g.vcursor_row++;
            cursor_moved = true;
            if(modifiers & GLFW_MOD_SHIFT) {
                nmterm_yscroll(st->terminal, 1);
            }
            break;

        case GLFW_KEY_J:
        case GLFW_KEY_LEFT:
            g.vcursor_col--;
            cursor_moved = true;
            break;

        case GLFW_KEY_L:
        case GLFW_KEY_RIGHT:
            g.vcursor_col++;
            cursor_moved = true;
            break;
    }


    if(cursor_moved && st->terminal->select.active) {
        nmterm_select_update(st->terminal, g.vcursor_col, g.vcursor_row);
    }

}

void module_loaded(size_t module_idx) {
    Nemi* st = nmt_getst();

    g.module_idx = module_idx;

    nmt_assign_module_keybind(st, module_idx, 
            keybind_toggle_fn, keybind_toggle, ARRAY_LEN(keybind_toggle));
}


void module_quit() {
}

