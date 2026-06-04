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
#ifndef NEMI_H
#define NEMI_H

#include <stdint.h>
#include "leaf/leaf.h"


#include "nemi_config.h"
#include "terminal.h"
#include "log.h"
#include "nmt_string.h"
#include "module.h"
#include "common.h"

#define NEMI_VERSION_STR "0.00-1"

// TODO: Add these to config.
#define NEMI_TERMINALS_MAX 16
#define NEMI_KEYINBUF_MAX 8
#define NEMI_CHARINBUF_MAX 8
#define NEMI_MODULES_MAX 64


typedef struct NemiFilepaths_t {
    char* nemi_home;
    char* libnemi;
    char* fonts;
    char* configs;
    char* modules;
}
NemiFilepaths;


typedef struct Nemi_t {
    int flags;

    NemiFilepaths  filepaths;
    NemiConfig     cfg;
    LeafCtx*       lfctx;    
    LeafFont       font;

    NTerminal    terminals [NEMI_TERMINALS_MAX];
    NTerminal*   terminal; // Current terminal.
    NTerminal*   messages; // Points to terminals[0]
    NTerminal*   terminal_prev; // Previous current terminal.
    uint16_t     num_terminals;

    NModule*     modules;
    size_t       num_loaded_modules;
    ssize_t      inputfocus_module_idx; // Negative number means that no module has inputfocus.

    int win_rows;
    int win_cols;

    // User input.
    // 'last_...' input variables are set only for one frame.
    // So that means very simple if statement can be used
    // even in continuously executing loops, eg, frame update.
    int  last_key_in;        // see 'leaf/keyboard.h'
    char last_char_in;       //
    int  last_keymod_in;     // see 'leaf/keyboard.h'
    int  last_mouse_button;  // see 'leaf/mouse.h'
   
    float mouse_x;
    float mouse_y;

    // Input ring buffers
    int  key_inputs  [NEMI_KEYINBUF_MAX];  // TODO: Idk if these are actually needed anyway...
    char char_inputs [NEMI_CHARINBUF_MAX]; // ^

    //struct image         images [NEMI_IMAGES_MAX];

    double frame_time;       // Previous frame time. In seconds.
    double frame_time_begin; // Time at begin of the frame. In seconds.

    int term_cells_render_offset_x;
    int term_cells_render_offset_y;

    // Terminal framebuffer is separated
    // because terminals keep track of "front" and "back" cell buffers
    // to optimize rendering.
    // We dont want to that kind of thing for other stuff
    // because they may not be bound to cell coordinates.
    //LeafFramebuffer term_cells_framebuffer; // Terminals are rendered to this framebuffer.
    //LeafFramebuffer altrender_framebuffer;  // Anything else is rendered to this framebuffer.

}
Nemi;


Nemi* nmt_start_session(NemiFilepaths filepaths);
void  nmt_quit_session(Nemi* st);

Nemi* nmt_getst();

void nmt_load_all_modules(Nemi* st);
void nmt_zero_input_buffers(Nemi* st);
void nmt_push_key_input(Nemi* st, int key);
void nmt_push_char_input(Nemi* st, char ch);
void nmt_font_scale(Nemi* st, float offset);
void nmt_set_font_scale(Nemi* st, float scale);
void nmt_create_msg(Nemi* st, const char* msg, ...);
//int  nmt_load_image(Nemi* st, const char* filepath);
//void nmt_unload_image(struct image* img);
//bool nmt_key_down(Nemi* st, int key);

// Switch current terminal.
void nmt_switch_terminal_idx(Nemi* st, uint32_t index);
void nmt_switch_terminal_ptr(Nemi* st, NTerminal* term);

void nmt_update_frame(Nemi* st);


// Inputfocus means that if the module gains it.
// None of the terminals will receive key or character input events.
// And only the focused module will receive the input events.
// The inputfocus must be remembered to be freed.
bool nmt_is_module_inputfocus_available(Nemi* st);
bool nmt_module_gain_inputfocus(Nemi* st, size_t module_idx);
void nmt_module_free_inputfocus(Nemi* st, size_t module_idx);
void nmt_keyinput_events_for_modules(Nemi* st, int key, int mods);

size_t nmt_hash_glfwkeys(const int* keys, size_t num_keys);
void nmt_assign_module_keybind(Nemi* st, size_t module_idx, void(*fnptr)(), const int* keys, size_t num_keys);


// Clear rendered pixels.
void nmt_clear_region(Nemi* st, int x, int y, int w, int h);
// Convert column/row to window x/y position.
int nmt_coltox(Nemi* st, int col);
int nmt_rowtoy(Nemi* st, int row);

void nmt_init_default_config(Nemi* st);

//void restart_session(Nemi* st);
//void hotreload_session(Nemi* st);
//void nemi_recompile_src(Nemi* st);
//const char* nemi_get_clipboard_content(Nemi* st);


#endif
