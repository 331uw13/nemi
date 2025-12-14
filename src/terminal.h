#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include <vterm.h>

#include "vec2.h"
#include "string.h"




struct terminal {
    int          flags;
    int          master_fd;
    pid_t        pid;
    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;
    

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


struct nemi;

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols);
void             close_terminal(struct terminal* term);

void read_terminal         (struct nemi* st, struct terminal* term);
void render_terminal       (struct nemi* st, struct terminal* term);
void write_terminal        (struct terminal* term, char* buffer, size_t size);

void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_resize_event (struct nemi* st, struct terminal* term);

void terminal_init_palette        (struct nemi* st, struct terminal* term);

#endif
