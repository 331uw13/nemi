#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include "vterm.h"

#include "vec2.h"
#include "string.h"



enum terminal_type {
    ECHO_TERMINAL,
    SHELL_TERMINAL
};

struct terminal {
    int          flags;
    int          master_fd;
    pid_t        pid;
    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;
    enum terminal_type type;

    // Back and front cell buffers for knowing what cells to render
    // so we dont need to render already rendered cells again.
    VTermScreenCell* front_cell_buffer;
    VTermScreenCell* back_cell_buffer;

    // Rows that are marked as "dirty" are rendered again.
    // 'dirty_rows' memory capacity is resized along side
    // with back and front cell buffers.
    bool* dirty_rows; 

    bool is_altbuffer_active;

    int yscroll;
    //struct scrollback sb;
    
    int rows;
    int cols;
    int line_height;

    int cursor_col;
    int cursor_row;
    int cursor_old_col;
    int cursor_old_row;

    float blink_timer;

    // TODO: This needs fixing.
    // Users can hide terminal's cells.
    // This is done because then they can render
    // anything they want from scripts without
    // the terminal's cells being in the way.
    bool* hidden_cells;
};

enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};


struct nemi;

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols, enum terminal_type term_type);
void             close_terminal(struct terminal* term);

void terminal_read         (struct nemi* st, struct terminal* term);
void terminal_render       (struct nemi* st, struct terminal* term);
void terminal_write        (struct terminal* term, enum term_write_target target, char* fmt, ...);
void update_terminal_blink_timer  (struct nemi* st, struct terminal* term);
void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_resize_event (struct nemi* st, struct terminal* term);
void terminal_handle_altbuffer_change_event(struct nemi* st, struct terminal* term);
void terminal_hide_cells          (struct terminal* term, bool hidden, int col, int row, int width, int height);
void terminal_init_palette        (struct nemi* st, struct terminal* term);
char terminal_get_char            (struct terminal* term, int column, int row);
int  terminal_get_cursor_x        (struct terminal* term);
int  terminal_get_cursor_y        (struct terminal* term);
void terminal_set_cell_custom_bg  (struct terminal* term, VTermPos pos, int hex_rgb_color);
void terminal_set_cell_custom_fg  (struct terminal* term, VTermPos pos, int hex_rgb_color);
void terminal_clear_cell_custom_bg (struct terminal* term, VTermPos pos);
void terminal_clear_cell_custom_fg (struct terminal* term, VTermPos pos);
void terminal_set_cell_custom_attrs  (struct terminal* term, VTermPos pos, int attrs);
void terminal_clear_cell_custom_attrs(struct terminal* term, VTermPos pos);
void terminal_set_row_dirty       (struct terminal* term, int row); // Causes a row to be re-rendered.
void terminal_yscroll             (struct nemi* st, struct terminal* term, int offset);
void terminal_yscroll_to          (struct nemi* st, struct terminal* term, int point);
// Available type is "block"(block select) or "normal".
// The reason why the type is string because perl doesnt have enums
// and writing only integer as the type is not as readable.
void terminal_copy_to_clipboard(struct nemi* st, struct terminal* term, const char* type,
                                    int start_col, int start_row, int end_col, int end_row);


#endif
