
#include "nemi.h"
#include "leaf/keyboard.h"
#include "leaf/mouse.h"



static struct {
    float grab_begin_x;
    float grab_begin_y;
    bool  grab_active;
}
g;



void module_event_mouse_scroll(int direction) {
    Nemi* nemi = nmt_getst();
    nmterm_yscroll(nemi->terminal, -direction);
}

void module_event_mouse_moved(float new_x, float new_y) {
    Nemi* nemi = nmt_getst();

    float mouse_delta_x = nemi->mouse_x - new_x;
    float mouse_delta_y = nemi->mouse_y - new_y;

    if(g.grab_active) {
        nmterm_box_move(nemi->terminal,
                nemi->terminal->box.x - mouse_delta_x,
                nemi->terminal->box.y - mouse_delta_y);
    }

}

void module_event_render() {
    Nemi* nemi = nmt_getst();
   
    bool grab_keys_down = 
        leaf_key_down(KEYBOARD_KEY_LEFT_SHIFT) &&
        leaf_mouse_down(MOUSE_LEFT);

    if(grab_keys_down && !g.grab_active) {
        g.grab_active = true;
        g.grab_begin_x = nemi->mouse_x;
        g.grab_begin_y = nemi->mouse_y;
    }
    else
    if(!grab_keys_down) {
        g.grab_active = false;
    }
}

void module_loaded(size_t module_idx) {
}

void module_quit() {
}





