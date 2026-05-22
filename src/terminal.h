#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include "vterm.h"

#include "string.h"



enum terminal_type {
    ECHO_TERMINAL,
    SHELL_TERMINAL
};


typedef enum NTerminalSelectRegMode_e {
    SREG_MODE_NORMAL,
    SREG_MODE_LINE,
    SREG_MODE_BLOCK
}
NTerminalSelectRegMode;

typedef struct NTerminalSelectReg_t {
    bool active;
    NTerminalSelectRegMode mode;
    int col_beg;
    int row_beg;
    int col_end;
    int row_end;
}
NTerminalSelectReg;

typedef struct NTerminal_t {
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


    NTerminalSelectReg select;

    float blink_timer;

    // TODO: This needs fixing.
    // Users can hide terminal's cells.
    // This is done because then they can render
    // anything they want from scripts without
    // the terminal's cells being in the way.
    //bool* hidden_cells;
}
NTerminal;

enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};



typedef struct Nemi_t Nemi;

NTerminal* nmterm_spawn(Nemi* st, int rows, int cols, enum terminal_type term_type);
void             close_terminal(NTerminal* term);

void nmterm_read         (Nemi* st, NTerminal* term);
void nmterm_render       (Nemi* st, NTerminal* term);
void nmterm_write        (NTerminal* term, enum term_write_target target, char* fmt, ...);
void nmterm_update_blink_timer  (Nemi* st, NTerminal* term);
void nmterm_handle_char_event   (Nemi* st, NTerminal* term);
void nmterm_handle_key_event    (Nemi* st, NTerminal* term);
void nmterm_handle_resize_event (Nemi* st, NTerminal* term);
void nmterm_handle_altbuffer_change_event(Nemi* st, NTerminal* term);
//void nmterm_hide_cells          (NTerminal* term, bool hidden, int col, int row, int width, int height);
void nmterm_init_palette        (Nemi* st, NTerminal* term);
char nmterm_get_char            (NTerminal* term, int column, int row);
void nmterm_set_cell_custom_bg  (NTerminal* term, VTermPos pos, int hex_rgb_color);
void nmterm_set_cell_custom_fg  (NTerminal* term, VTermPos pos, int hex_rgb_color);
void nmterm_clear_cell_custom_bg (NTerminal* term, VTermPos pos);
void nmterm_clear_cell_custom_fg (NTerminal* term, VTermPos pos);
void nmterm_set_cell_custom_attrs   (NTerminal* term, VTermPos pos, int attrs);
void nmterm_clear_cell_custom_attrs (NTerminal* term, VTermPos pos);
void nmterm_set_row_dirty           (NTerminal* term, int row); // Causes a row to be re-rendered.
void nmterm_yscroll                 (NTerminal* term, int offset);
void nmterm_yscroll_to              (NTerminal* term, int point);
void nmterm_select_begin            (NTerminal* term, int col_begin, int row_begin);
void nmterm_select_update           (NTerminal* term, int col, int row);
void nmterm_select_end              (NTerminal* term);
void nmterm_select_process(Nemi* st, NTerminal* term, void(*callback)(Nemi* st, NTerminal* term, int row, int col_beg, int row_length));
//void nmterm_select_render           (Nemi* st, NTerminal* term);
int  nmterm_get_row_length          (NTerminal* term, int row);

// TODO: MOve this:
NTerminalSelectReg nmt_orientate_select_region(const NTerminalSelectReg* reg);
/*
// Available type is "block"(block select) or "normal".
// The reason why the type is string because perl doesnt have enums
// and writing only integer as the type is not as readable.
void nmterm_copy_to_clipboard(Nemi* st, NTerminal* term, const char* type,
                                    int start_col, int start_row, int end_col, int end_row);
*/


void nmterm_clear        (NTerminal* term);
void nmterm_clear_row    (NTerminal* term, int row);
void nmterm_clear_row_part(NTerminal* term, int row, int column_begin, int column_end);
void nmterm_mv_cursor    (NTerminal* term, int row, int column);
void nmterm_mv_putchr    (NTerminal* term, int row, int column, char chr);
void nmterm_mv_putstr_nullterm  (NTerminal* term, int row, int column, char* str);
void nmterm_mv_putstrn          (NTerminal* term, int row, int column, char* str, size_t len);



#endif
