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
#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include "vterm.h"

#include "leaf/framebuffer.h"
#include "string.h"
#include "select_region.h"


enum terminal_type {
    ECHO_TERMINAL,
    SHELL_TERMINAL
};

// The terminal box is the final position where
// it will be rendered. Scale is just font scale, but it basically scales the whole thing.
typedef struct NTerminalBox_t {
    float scale;
    int x;
    int y;
}
NTerminalBox;


typedef struct NTerminal_t {
    LeafFramebuffer cell_framebuffer; // Framebuffer for cells.
    LeafFramebuffer arbt_framebuffer; // Framebuffer for "arbitrary draws".
    NSelectRegion select;
    NTerminalBox  box;

    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;

    // Back and front cell buffers for knowing what cells to render
    // so we dont need to render already rendered cells again.
    VTermScreenCell* front_cell_buffer;
    VTermScreenCell* back_cell_buffer;

    // Rows that are marked as "dirty" are rendered again.
    // 'dirty_rows' memory capacity is resized along side
    // with back and front cell buffers.
    bool* dirty_rows; 

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
    
    enum terminal_type type;
    bool is_altbuffer_active;

    int          flags;
    int          master_fd;
    pid_t        pid;
}
NTerminal;

enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};



typedef struct Nemi_t Nemi;

NTerminal* nmterm_spawn(Nemi* st, int rows, int cols, enum terminal_type term_type);
void       nmterm_close(NTerminal* term);

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
void nmterm_clear_screen_row        (Nemi* st, NTerminal* term, int row);
void nmterm_yscroll                 (NTerminal* term, int offset);
void nmterm_yscroll_to              (NTerminal* term, int point);
int  nmterm_get_row_length          (NTerminal* term, int row);
void nmterm_box_move                (NTerminal* term, int new_box_x, int new_box_y);

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
