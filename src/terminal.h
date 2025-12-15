#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include <vterm.h>

#include "vec2.h"
#include "string.h"



struct scrollback_row {
    VTermScreenCell* cells;
    uint32_t         num_cells;
};

struct scrollback {
    struct scrollback_row* rows;
    size_t num_rows;
    size_t num_rows_max;
    int    offset;
};

struct terminal {
    int          flags;
    int          master_fd;
    pid_t        pid;
    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;

    struct scrollback sb;
    
    int rows;
    int cols;
    int line_height;


    // Visual mode.
    struct {
        bool enabled;

        int  curs_row;
        int  curs_col;

        // Select region.
        bool sel; 
        int  sel_start_row;
        int  sel_start_col;
    }
    vmode;
};


enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};

struct nemi;

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols);
void             close_terminal(struct terminal* term);

void read_terminal         (struct nemi* st, struct terminal* term);
void render_terminal       (struct nemi* st, struct terminal* term);
void write_term            (struct terminal* term, enum term_write_target target, char* fmt, ...);
void terminal_scroll       (struct terminal* term, int offset);
void terminal_set_scroll   (struct terminal* term, int scroll);
void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_resize_event (struct nemi* st, struct terminal* term);

void terminal_init_palette        (struct nemi* st, struct terminal* term);

#endif
