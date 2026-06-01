#include "nemi.h"
#include "leaf/keyboard.h"

static const int keybind_toggle[] = {
    KEYBOARD_KEY_LEFT_CONTROL,
    KEYBOARD_KEY_K
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

    leaf_draw_rect(
            nmt_coltox(st, g.vcursor_col),
            nmt_rowtoy(st, g.vcursor_row - st->terminal->yscroll) + st->font.char_height,
            st->font.char_width,
            2,
            (RGBColor){ 10, 160, 10 });
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

        case KEYBOARD_KEY_S:
            if(st->terminal->select.active) {
                st->terminal->select.active = false;
                //nmt_select_end(st->terminal);
            }
            else {
                nmt_select_begin(&st->terminal->select, g.vcursor_col, g.vcursor_row);
            }
            break;

        case KEYBOARD_KEY_I:
        case KEYBOARD_KEY_UP:
            g.vcursor_row--;
            cursor_moved = true;
            if(modifiers & GLFW_MOD_SHIFT) {
                nmterm_yscroll(st->terminal, -1);
            }
            break;

        case KEYBOARD_KEY_K:
        case KEYBOARD_KEY_DOWN:
            g.vcursor_row++;
            cursor_moved = true;
            if(modifiers & GLFW_MOD_SHIFT) {
                nmterm_yscroll(st->terminal, 1);
            }
            break;

        case KEYBOARD_KEY_J:
        case KEYBOARD_KEY_LEFT:
            g.vcursor_col--;
            cursor_moved = true;
            break;

        case KEYBOARD_KEY_L:
        case KEYBOARD_KEY_RIGHT:
            g.vcursor_col++;
            cursor_moved = true;
            break;
    }


    if(cursor_moved && st->terminal->select.active) {
        nmt_select_move(&st->terminal->select, g.vcursor_col, g.vcursor_row);
    }

}

void module_loaded(size_t module_idx) {
    Nemi* st = nmt_getst();

    g.module_idx = module_idx;
    nmt_assign_module_keybind(st, module_idx, keybind_toggle_fn, keybind_toggle, ARRAY_LEN(keybind_toggle));
}

/*
void module_quit() {
}
*/

