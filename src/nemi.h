#ifndef NEMI_H
#define NEMI_H

#include <stdint.h>


#include "leaf/leaf.h"
#include "nemi_config.h"
#include "terminal.h"
#include "perl_script.h"
#include "script_keybinds.h"
#include "log.h"
#include "string.h"

#define NEMI_VERSION_STR "0.00-0"

// Flags for 'struct nemi':
#define FLG_HOTRELOADING_SUPPORTED   (1 << 1)
#define FLG_RESTARTING_SUPPORTED     (1 << 2)
#define FLG_LOADER_HOTRELOAD_SESSION (1 << 3)
#define FLG_LOADER_RESTART_SESSION   (1 << 4)
#define FLG_SCRIPTDRAW_ADJUSTPOS_TO_SCROLL (1 << 5)

// TODO: Add these to config.
#define NEMI_TERMINALS_MAX 16
#define NEMI_KEYINBUF_MAX 8
#define NEMI_CHARINBUF_MAX 8
#define NEMI_SCRIPTS_MAX 32
#define NEMI_RENDERBUFS_MAX 32
#define NEMI_MSG_LINES_MAX 64
#define NEMI_SCRIPTS_KEYBIND_KEYS_MAX 16
#define NEMI_IMAGES_MAX 32



struct nemi_filepaths {
    char* nemi_home;
    char* libnemi;
    char* fonts;
    char* configs;
    char* scripts;
};

struct image {
    int width;
    int height;
    int handle; // Index to 'nemi.images' array.
    uint32_t texture;
};

typedef struct Nemi_t {
    int flags;

    struct nemi_filepaths filepaths;
    struct leaf_ctx_t* lfctx;    
    struct font_t      font;
    struct nemi_config cfg;

    NTerminal    terminals [NEMI_TERMINALS_MAX];
    NTerminal*   terminal; // Current terminal.
    NTerminal*   messages; // Points to terminals[1]
    NTerminal*   terminal_prev; // Previous current terminal.
    uint16_t           num_terminals;    

    int win_rows;
    int win_cols;

    // User input.
    int  last_key_in;
    char last_char_in;
    int  last_keymod_in;
   
    // Input ring buffers
    int  key_inputs  [NEMI_KEYINBUF_MAX];  // TODO: Idk if these are actually needed anyway...
    char char_inputs [NEMI_CHARINBUF_MAX]; // ^

    PerlScript  scripts [NEMI_SCRIPTS_MAX];
    size_t      num_scripts;

    struct image         images [NEMI_IMAGES_MAX];

    double frame_time;
    double frame_time_begin;

    // When scripts want to make user input
    // to the terminal ignored we will keep track of
    // how many scripts ignored it
    // because if another script ignores input and another unignores it
    // it will not work as expected.
    // FIXME: The counters get confused very rarely and locks the input forever.
    // TODO: Try to create "focus" system for scripts (?)
    int term_ignore_char_input_counter;
    int term_ignore_key_input_counter;


    // Terminal framebuffer is separated
    // because terminals keep track of "front" and "back" cell buffers
    // to optimize rendering.
    // We dont want to that kind of thing for other stuff
    // because they may not be bound to cell coordinates.
    struct framebuffer term_cells_framebuffer; // Terminals are rendered to this framebuffer.
    struct framebuffer altrender_framebuffer;  // Anything else is rendered to this framebuffer.

}
Nemi;


Nemi* start_session(struct nemi_filepaths filepaths);
void         quit_session(Nemi* st);
void         prepare_from_hotreload(Nemi* st); // Some global variables must be set again after hotreloading.

//void         restart_session(Nemi* st);

Nemi* get_state(); 

void zero_input_buffers(Nemi* st);
void push_key_input(Nemi* st, int key);
void push_char_input(Nemi* st, char ch);
void font_scale(Nemi* st, float offset);
void set_font_scale(Nemi* st, float scale);
void create_msg(Nemi* st, const char* msg, ...);
int  load_image(Nemi* st, const char* filepath);
void unload_image(struct image* img);

// Switch current terminal.
void switch_terminal(Nemi* st, uint32_t index);
void switch_terminal_ptr(Nemi* st, NTerminal* term);


//void begin_frame(Nemi* st);
//void end_frame(Nemi* st);
void update_frame(Nemi* st);

bool key_down(Nemi* st, int key);


// 'event_num' corresponds to REG_EVENT... defined in "script.h"
// 'arg_types' should be array of characters matching the variable type's
// first letter, respectively to variadic arguments.
// For example if arguments to function is int, int, float, float,
// then arg_types should be "iiff".
// TODO: Not all types are currently supported.
void trigger_event_for_scripts(Nemi* st, int event_num, const char* arg_types, ...);

// Clear rendered pixels.
void clear_region(Nemi* st, int x, int y, int w, int h);

// Convert column/row to window x/y position.
int coltox(Nemi* st, int col);
int rowtoy(Nemi* st, int row);

//int new_renderbuf(Nemi* st, int num_nodes_max);
void init_default_config(Nemi* st);


void nemi_help(Nemi* st, const char* what);
void nemi_message_script_keybinds(Nemi* st, const char* script_name);

void restart_session(Nemi* st);
void hotreload_session(Nemi* st);
//void nemi_recompile_src(Nemi* st);

const char* nemi_get_clipboard_content(Nemi* st);


#endif
