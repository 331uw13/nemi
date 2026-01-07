#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include "vterm.h"

#include "vec2.h"
#include "string.h"



struct scrollback_row {
    VTermScreenCell* cells;
    uint32_t         num_cells;
    uint32_t         num_cells_alloc;
};

struct scrollback {
    struct scrollback_row* rows;
    size_t num_rows;
    size_t num_rows_max;
    int    offset;
};

enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};


struct terminal {
    int          flags;
    int          master_fd;
    pid_t        pid;
    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;
    bool         is_altscreen;

    struct scrollback sb;
    
    int rows;
    int cols;
    int line_height;

    float blink_timer;

    // Users can hide terminal's cells.
    // This is done because then they can render
    // anything they want from scripts without
    // the terminal's cells being in the way.
    bool* hidden_cells;
};


struct nemi;

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols, const char* shell);
void             close_terminal(struct terminal* term);

void read_terminal         (struct nemi* st, struct terminal* term);
void render_terminal       (struct nemi* st, struct terminal* term);
void write_term            (struct terminal* term, enum term_write_target target, char* fmt, ...);
void terminal_scroll       (struct terminal* term, int offset);
void terminal_set_scroll   (struct terminal* term, int scroll);
void update_terminal_blink_timer  (struct nemi* st, struct terminal* term);
void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_resize_event (struct nemi* st, struct terminal* term);
void terminal_handle_altscreen_change_event(struct nemi* st, struct terminal* term);
void terminal_hide_cells          (struct terminal* term, bool hidden, int col, int row, int width, int height);
void terminal_init_palette        (struct nemi* st, struct terminal* term);
char terminal_get_char            (struct terminal* term, int column, int row);
int  terminal_get_cursor_x        (struct terminal* term);
int  terminal_get_cursor_y        (struct terminal* term);

#endif
