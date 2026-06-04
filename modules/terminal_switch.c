
// Test module to allow switching between terminals.
// NOTE: Work in progress.
// -------------------------------------------------


#include "nemi.h"
#include "leaf/keyboard.h"
#define ACTIVE_TIME_TIMEOUT_SEC 1.0f


static struct Module_t {
    size_t idx;
    float  active_timer_sec;
}
module;


static const int KEYBIND_SWITCH[] = {
    KEYBOARD_KEY_LEFT_SHIFT,
    KEYBOARD_KEY_LEFT_ALT,
    KEYBOARD_KEY_M
};


void reset_timeout() {
    module.active_timer_sec = ACTIVE_TIME_TIMEOUT_SEC;
}

bool is_module_active() {
    return (module.active_timer_sec > 0);
}

void module_event_render() {
    Nemi* nemi = nmt_getst();

    if(is_module_active()) {
        module.active_timer_sec -= nemi->frame_time;
    
        if(!is_module_active()) {
            nmt_module_free_inputfocus(nmt_getst(), module.idx);
        }

        int box_width = 10;
        int box_height = 20;

        leaf_draw_rect
        (
            nmt_coltox(nemi, nemi->win_cols - box_width),
            nmt_rowtoy(nemi, 0),
            nemi->font.char_width * box_width,
            nemi->font.char_height * box_height,
            (RGBColor) {
                50, 50, 50
            }
        );
    }


    //logprintf(LOG_INFO, "%f", module.active_timer_sec);
}

void module_event_key_input(int key, int mods) {
    if(is_module_active()) {
        reset_timeout();
    }
}


void kb_switch() {
    reset_timeout();
    nmt_module_gain_inputfocus(nmt_getst(), module.idx);
}


void module_loaded(size_t module_idx) {

    module.active_timer_sec = 0;
    module.idx = module_idx;



    Nemi* nemi = nmt_getst();

    nmt_assign_module_keybind
    (
        nemi,
        module_idx,
        kb_switch,
        KEYBIND_SWITCH,
        ARRAY_LEN( KEYBIND_SWITCH )
    );

}

void module_quit() {
}







