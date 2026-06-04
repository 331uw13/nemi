
#include "nemi.h"




void module_event_mouse_scroll(int direction) {
    Nemi* nemi = nmt_getst();
    nmterm_yscroll(nemi->terminal, -direction);
}

void module_event_mouse_moved(float new_x, float new_y) {
}

void module_event_render() {
}

void module_loaded(size_t module_idx) {
}

void module_quit() {
}





