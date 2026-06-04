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
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <pty.h>
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include "terminal.h"
#include "nemi.h"
#include "common.h"
#include "memory.h"

#include "leaf/keyboard.h"

static
const char* get_terminaltype_shell(Nemi* st, enum terminal_type term_type) {
    if(term_type == SHELL_TERMINAL) {
        return st->cfg.main.shell;
    }
    else
    if(term_type == ECHO_TERMINAL) {
        return "/bin/cat";
    }

    return NULL;
}



static
void copy_cell(VTermScreenCell* src, VTermScreenCell* dest) {
    dest->width = src->width;
    dest->attrs = src->attrs;

    /*memset(&dest->fg, 0, sizeof(VTermColor));
    memset(&dest->bg, 0, sizeof(VTermColor));
    memcpy(&dest->bg, &src->bg, sizeof(VTermColor));
    memcpy(&dest->fg, &src->fg, sizeof(VTermColor));
    */

    dest->fg = src->fg;
    dest->bg = src->bg;
    memcpy(dest->chars, src->chars, sizeof(src->chars));
}


static inline
bool do_vterm_colors_match(VTermColor* color_a, VTermColor* color_b) {
    
    if(color_a->type != color_b->type) {
        return false;
    }

    if(VTERM_COLOR_IS_RGB(color_a)
    && VTERM_COLOR_IS_RGB(color_b)) {
        if(color_a->rgb.red   != color_b->rgb.red 
        || color_a->rgb.green != color_b->rgb.green
        || color_a->rgb.blue  != color_b->rgb.blue) {
            return false;
        }
    }
    
    if(VTERM_COLOR_IS_INDEXED(color_a)
    && VTERM_COLOR_IS_INDEXED(color_b)) {
        if(color_a->indexed.idx != color_b->indexed.idx) {
            return false;
        }
    }
    
    return true;
}

static inline
bool do_cells_match(VTermScreenCell* cell_a, VTermScreenCell* cell_b) {
    if(cell_a->width != cell_b->width) {
        return false;
    }

    if(cell_a->attrs.bold      != cell_b->attrs.bold 
    || cell_a->attrs.underline != cell_b->attrs.underline
    || cell_a->attrs.italic    != cell_b->attrs.italic
    || cell_a->attrs.blink     != cell_b->attrs.blink
    || cell_a->attrs.reverse   != cell_b->attrs.reverse
    || cell_a->attrs.conceal   != cell_b->attrs.conceal
    || cell_a->attrs.strike    != cell_b->attrs.strike
    || cell_a->attrs.font      != cell_b->attrs.font
    || cell_a->attrs.dwl       != cell_b->attrs.dwl
    || cell_a->attrs.dhl       != cell_b->attrs.dhl
    || cell_a->attrs.small     != cell_b->attrs.small
    || cell_a->attrs.baseline  != cell_b->attrs.baseline) {
        return false;
    }

    if(!do_vterm_colors_match(&cell_a->fg, &cell_b->fg)) {
        return false;
    }

    if(!do_vterm_colors_match(&cell_a->bg, &cell_b->bg)) {
        return false;
    }
    
    for(size_t i = 0; i < VTERM_MAX_CHARS_PER_CELL; i++) {
        if(cell_a->chars[i] != cell_b->chars[i]) {
            return false;
        }
    }

    return true;
}
static 
void resize_nmterm_cell_buffers(NTerminal* term) {
    
    // TODO: Add error checking.

    size_t buffer_size = (term->cols * term->rows) * sizeof(VTermScreenCell);
    term->front_cell_buffer = realloc(term->front_cell_buffer, buffer_size);
    term->back_cell_buffer = realloc(term->back_cell_buffer, buffer_size);


    const size_t dirty_rows_size = term->rows * sizeof *term->dirty_rows;

    term->dirty_rows = realloc(term->dirty_rows, dirty_rows_size);
    
    // Set all rows as "dirty" to re-render the whole terminal.
    memset(term->dirty_rows, 1, dirty_rows_size);
}

NTerminal* nmterm_spawn(Nemi* st, int rows, int cols, enum terminal_type term_type) {
    if(st->num_terminals+1 >= NEMI_TERMINALS_MAX) {
        return NULL;
    }
    
    struct winsize ws = (struct winsize) {
        .ws_row = rows,
        .ws_col = cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

    NTerminal* term = &st->terminals[st->num_terminals++];
    term->type = term_type;
    term->pid = forkpty(&term->master_fd, NULL, NULL, &ws);

    if(term->pid == 0) {
        const char* shell = get_terminaltype_shell(st, term_type);
        if(!shell) {
            logprintf(LOG_ERROR, "Invalid terminal type!");
        }

        setenv("TERM", "xterm-256color", 1);
        execlp(shell, shell, NULL);
    }
    
    leaf_create_framebuffer(&term->cell_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    leaf_create_framebuffer(&term->arbt_framebuffer, st->lfctx->win_width, st->lfctx->win_height);

    term->rows = rows;
    term->cols = cols;
    term->front_cell_buffer = NULL;
    term->back_cell_buffer = NULL;
    term->dirty_rows = NULL;
    term->cursor_col = 0;
    term->cursor_row = 0;
    term->cursor_old_col = 0;
    term->cursor_old_row = 0;
    term->flags = 0;
    term->yscroll = 0;
    term->line_height = st->font.char_height + st->cfg.main.line_padding;
    term->blink_timer = 0;
    term->is_altbuffer_active = false;
    term->select.active = false;
    term->select.col_beg = 0;
    term->select.row_beg = 0;
    term->select.col_end = 0;
    term->select.row_end = 0;
    term->select.mode = SREG_MODE_NORMAL;

    term->box = (NTerminalBox) {
        .scale = 0.8f,
        .x = 0,
        .y = 0
    };

    //term->hidden_cells = calloc(term->cols * term->rows, sizeof *term->uhidden_cells);

    //init_scrollback_buffer(term);
    term->vt = vterm_new(rows, cols);
    
    // Even if UTF-8 is not supported by font renderer
    // at the moment. Some applications are completely unusable
    // if this is set to 'false'
    vterm_set_utf8(term->vt, true);

    term->vtscrn = vterm_obtain_screen(term->vt);
    term->vtstate = vterm_obtain_state(term->vt);

    //vterm_screen_set_putglyph_callback(term->vtscrn, vterm_putglyph_callback, term);
    //vterm_state_set_scroll_callback(term->vtstate, vterm_scroll_callback, term);
    vterm_screen_enable_reflow(term->vtscrn, true);
    vterm_screen_enable_altscreen(term->vtscrn, true);
    vterm_screen_reset(term->vtscrn, true);

    term->is_altbuffer_active = false;
    nmterm_init_palette(st, term);

    resize_nmterm_cell_buffers(term);


    logprintf(LOG_INFO, "Created %s terminal (%ix%i)", 
            (term_type == SHELL_TERMINAL) ? "shell" : "echo",
            term->rows,
            term->cols);
    
    if(term_type != ECHO_TERMINAL) {
        nmterm_write(term, TERM_WRITE_PTY, "clear\n");
    }
    return term;
}

void nmterm_close(NTerminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }
    
    leaf_free_framebuffer(&term->cell_framebuffer);
    leaf_free_framebuffer(&term->arbt_framebuffer);

    //freeif(term->hidden_cells);
    freeif(term->front_cell_buffer);
    freeif(term->back_cell_buffer);

    close(term->master_fd);
    term->master_fd = -1;
    
    vterm_free(term->vt);
    term->vt = NULL;
}


void nmterm_read(Nemi* st, NTerminal* term) {
    if(!term) {
        return;
    }

   
    struct pollfd pfd;
    pfd.fd = term->master_fd;
    pfd.events = POLLIN;
    nfds_t num_fds = 1;

    char tmp_buffer[256] = { 0 };

    const int timeout_ms = 10;
    ssize_t last_rd_len = sizeof(tmp_buffer);

    int times_read = 0;

    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv <= 0) {
            break;
        }
        
        memset(tmp_buffer, 0, last_rd_len);
        ssize_t rd_len = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
        if(rd_len <= 0) {
            break;
        }
            
        vterm_input_write(term->vt, tmp_buffer, rd_len);
        vterm_screen_flush_damage(term->vtscrn);
        last_rd_len = rd_len;
    
        times_read++;
        if(times_read > 200) {
            // Continue later.
            // We would otherwise block all input and rendering for a while.
            break; 
        }
    }


    bool current_altbuffer_status = vterm_screen_is_altscreen(term->vtscrn);
    if(current_altbuffer_status != term->is_altbuffer_active) {
        term->is_altbuffer_active = current_altbuffer_status;
        nmterm_handle_altbuffer_change_event(st, term);
    }
}



/*
static
void vtermcolor_to_leafcolor(VTermColor* c) {
    return (RGBColor) {
        c->rgb.red, c->rgb.green, c->rgb.blue
    };
}
*/

/*
static
bool* get_hidden_cell_status(NTerminal* term, int col, int row) {
    size_t index = row * term->cols + col;

    size_t num_cells = term->cols * term->rows;
    if(index >= num_cells) {
        return NULL;
    }

    return &term->hidden_cells[index];
}
*/
/*
void nmterm_hide_cells(NTerminal* term, bool hidden, int col, int row, int width, int height) {
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            bool* status = get_hidden_cell_status(term, x + col, y + row);
            if(!status) {
                continue;
            }

            *status = hidden;
        }
    }
}
*/


/*
void nmterm_process_select_region(Nemi* st, NTerminal* term, void(*callback)(Nemi* st, NTerminal* term, int row, int col_beg, int row_length)) {
    
    RGBColor color = (RGBColor) { 100, 100, 100 };


    NSelectRegion reg = nmt_select_orientate(&term->select);

    for(int row = reg.row_beg; row <= reg.row_end; row++) {

        switch(reg.mode) {
            case SREG_MODE_NORMAL:
                {
                    int real_row_length = nmterm_get_row_length(term, row);
                    int row_length = real_row_length;
                    if(row_length == 0) {
                        break;
                    }

                    int col_beg = 0;

                    if(row == reg.row_beg && row == reg.row_end) {
                        if(reg.col_beg < reg.col_end) {
                            col_beg = reg.col_beg;
                            row_length = reg.col_end - reg.col_beg;
                        }   
                        else {
                            col_beg = reg.col_end;
                            row_length = reg.col_beg - reg.col_end;
                        }
                    }
                    else
                    if(row == reg.row_beg) {
                        col_beg = reg.col_beg;
                        row_length -= col_beg;
                    }
                    else
                    if(row == reg.row_end) {
                        row_length -= (row_length - reg.col_end);
                    }

                    if(row_length <= 0) {
                        break;
                    }

                    if(row_length > real_row_length) {
                        row_length = real_row_length;
                    }

                    callback(st, term, row, col_beg, row_length);
                }
                break;
            
            case SREG_MODE_LINE:
                {
                    int row_length = nmterm_get_row_length(term, row);
                    int col_beg = 0;

                    callback(st, term, row, col_beg, row_length);
                }
                break;

            case SREG_MODE_BLOCK:
                {
                    int row_length = 0; 
                    int col_beg = 0;
                    if(reg.col_beg < reg.col_end) {
                        col_beg = reg.col_beg;
                        row_length = reg.col_end - reg.col_beg;
                    }   
                    else {
                        col_beg = reg.col_end;
                        row_length = reg.col_beg - reg.col_end;
                    }

                    callback(st, term, row, col_beg, row_length);
                }
                break;
        }
    }
}
*/

int nmterm_get_row_length(NTerminal* term, int row) {
    int len = 0;
    for(int i = 0; i < term->cols; i++) {
        char ch = nmterm_get_char(term, i, row);
        if(ch < 0x20 || ch > 0x7E) {
            break;
        }
        len++;
    }

    return len;
}


static
RGBColor convert_cell_color_or_default(NTerminal* term, VTermColor* vtcolor, RGBColor default_color) {
    RGBColor ret_color = default_color;
    if(VTERM_COLOR_IS_INDEXED(vtcolor)) {
        vterm_state_convert_color_to_rgb(term->vtstate, vtcolor);
    }

    if(VTERM_COLOR_IS_RGB(vtcolor)) {
        ret_color.r = vtcolor->rgb.red;
        ret_color.g = vtcolor->rgb.green;
        ret_color.b = vtcolor->rgb.blue;
    }

    return ret_color;
}

static
bool render_cell(Nemi* st, NTerminal* term, VTermScreenCell* cell, VTermPos pos) {
   
    if(cell->chars[0] == 0) {
        return false;
    }

    /*
    // Unicode is not supported at least for now.
    // So just skip anything that is not ascii character.
    if(cell->chars[0] < 0x20 || cell->chars[0] > 0x7E) {
        return false;
    }
    */

    int char_x = nmt_coltox(st, pos.col) * st->cfg.font.char_spacing;
    int char_y = nmt_rowtoy(st, pos.row);

    //asm("int3");

    bool draw_bg = false;

    RGBColor fg_color = convert_cell_color_or_default(term, &cell->fg, st->cfg.colors[NEMI_COLOR_FG]);
    RGBColor bg_color = convert_cell_color_or_default(term, &cell->bg, st->cfg.colors[NEMI_COLOR_BG]);


    if(bg_color.r != st->cfg.colors[NEMI_COLOR_BG].r
    || bg_color.g != st->cfg.colors[NEMI_COLOR_BG].g
    || bg_color.b != st->cfg.colors[NEMI_COLOR_BG].b) {
        draw_bg = true;
    }

   

    if(cell->attrs.blink) {
        fg_color = leaf_color_lerp(fg_color, bg_color, term->blink_timer);
    }

    if(cell->attrs.reverse) {
        RGBColor tmp = fg_color;
        fg_color = bg_color;
        bg_color = tmp;

        draw_bg = true;
    }

    if(cell->attrs.bold) {
        st->font.bold = 2.0f;
    }

    st->font.italic = (cell->attrs.italic) ? st->cfg.font.italic_tilt : 0.0f;
    


    // Font renderer dont support UTF-8 at the moment.
    char cell_char = cell->chars[0];
    if(cell_char < 0x20 || cell_char > 0x7E) {
        fg_color = (RGBColor) { 255, 20, 20 };
        cell_char = '?';
    }


    if(draw_bg) {
        leaf_draw_rect(
                char_x, 
                char_y,
                st->font.char_width,
                st->font.char_height + st->cfg.main.line_padding,
                bg_color);
    }

    leaf_set_font_scale(&st->font, term->box.scale);
    leaf_set_font_color(&st->font, fg_color);
    leaf_draw_char(&st->font, char_x, char_y, cell_char);


    if(cell->attrs.underline) {
        leaf_draw_rect(
                char_x,
                char_y + st->font.char_height + st->cfg.font.underline_offset,
                st->font.char_width,
                st->cfg.font.underline_height,
                fg_color);
    }

    st->font.bold = 0.0f;
    st->font.italic = 0.0f;
    return true;
}


static
void clear_cell(Nemi* st, int col, int row) {
    int x = nmt_coltox(st, col) * st->cfg.font.char_spacing;
    int y = nmt_rowtoy(st, row);
    nmt_clear_region(st, x, y, 
            st->font.char_width,
            st->font.char_height); 
}

static
void nmterm_render_cursor(Nemi* st, NTerminal* term) {
    VTermPos vtcurs_pos = (VTermPos){ 0, 0 };
    vterm_state_get_cursorpos(term->vtstate, &vtcurs_pos);


    int cursor_draw_row = vtcurs_pos.row - term->yscroll;
    int cursor_draw_col = vtcurs_pos.col;

    if(cursor_draw_row < 0 || cursor_draw_row >= term->rows) {
        return;
    }
    if(cursor_draw_col < 0 || cursor_draw_col >= term->cols) {
        return;
    }

    int cursor_draw_x = nmt_coltox(st, cursor_draw_col) * st->cfg.font.char_spacing;
    int cursor_draw_y = nmt_rowtoy(st, cursor_draw_row);

    VTermScreenCell old_cursor_cell;
    VTermPos old_cursor_pos = (VTermPos){
        term->cursor_old_row,
        term->cursor_old_col
    };

    if(vterm_screen_get_cell(term->vtscrn, old_cursor_pos, &old_cursor_cell)) {
        old_cursor_pos.row -= term->yscroll;
        render_cell(st, term, &old_cursor_cell, old_cursor_pos);
    }

    nmt_clear_region(st,
            nmt_coltox(st, term->cursor_old_col), 
            nmt_rowtoy(st, term->cursor_old_row - term->yscroll),
            st->font.char_width,
            st->font.char_height);
    //clear_cell(st, term->cursor_old_col, term->cursor_old_row - term->yscroll);


    RGBColor cursor_color = st->cfg.colors[NEMI_COLOR_CURSOR];
    cursor_color = leaf_color_lerp(cursor_color, st->cfg.colors[NEMI_COLOR_BG], term->blink_timer);


    leaf_draw_rect(
            cursor_draw_x,
            cursor_draw_y,
            st->font.char_width,
            st->font.char_height,
            cursor_color);
    

    term->cursor_old_row = term->cursor_row;
    term->cursor_old_col = term->cursor_col;

    term->cursor_row = vtcurs_pos.row;
    term->cursor_col = vtcurs_pos.col;

}


void nmterm_set_row_dirty(NTerminal* term, int row) {
    if(row < 0) {
        return;
    }
    if(row >= term->rows) {
        return;
    }

    term->dirty_rows[row] = true;
}

void nmterm_clear_screen_row(Nemi* st, NTerminal* term, int row) {
    
    LeafFramebuffer* prev_fb = leaf_get_active_framebuffer();
    leaf_use_framebuffer(&term->cell_framebuffer);

    nmt_clear_region(st,
            0, 
            nmt_rowtoy(st, row),
            st->font.char_width * (term->cols + 1),
            st->font.char_height + st->cfg.main.line_padding);
    
    leaf_use_framebuffer(prev_fb);
}

void nmterm_render(Nemi* st, NTerminal* term) {
    vterm_get_size(term->vt, &term->rows, &term->cols);
    if(vterm_screen_is_altscreen(term->vtscrn)) {
        term->yscroll = 0;
    }
    
    const int rows_end = term->rows - MAX_VALUE(term->yscroll, 0);

    if(rows_end < 0) {
        return;
    }

    if(rows_end > term->rows) {
        logprintf(LOG_ERROR, "Failed to calculate terminal rows end. Did the MAX_VALUE macro change?");
        asm("int3");
        abort();
    }

    // Update front buffer.
    for(int row = 0; row < rows_end; row++) {
        for(int col = 0; col < term->cols; col++) {
            int actual_row = row + term->yscroll;
     
            VTermScreenCell* back_cell  = &term->back_cell_buffer  [col + row * term->cols];
            VTermScreenCell* front_cell = &term->front_cell_buffer [col + row * term->cols];

            // Get up to date cell to 'front_cell'
            if(!vterm_screen_get_cell(term->vtscrn, (VTermPos){ actual_row, col }, front_cell)) {
                continue;
            }

            if(!do_cells_match(front_cell, back_cell)) {
                term->dirty_rows[row] = true;
                copy_cell(front_cell, back_cell);
            }
        }
    }



    // Render cells, but only ones which changed.
    int cleared_rows = 0;
    
    //printf("%s: scroll:%i - rows_end:%i\n", __func__, term->yscroll, rows_end);
    for(int row = 0; row < rows_end; row++) {
        if(!term->dirty_rows[row]) {
            continue;
        }

        nmterm_clear_screen_row(st, term, row);

        for(int col = 0; col < term->cols; col++) {
            VTermScreenCell* front_cell = &term->front_cell_buffer[col + row * term->cols];
            render_cell(st, term, front_cell, (VTermPos){ row, col });
        }

        cleared_rows++;
    }
        


    //printf("Cleared rows: %i\n", cleared_rows);

    if(term->type != ECHO_TERMINAL) {
        nmterm_render_cursor(st, term);
    }
   
    memset(term->dirty_rows, 0, term->rows * sizeof *term->dirty_rows);
}

void nmterm_update_blink_timer(Nemi* st, NTerminal* term) {
    // https://en.wikipedia.org/wiki/Triangle_wave
    // Here triangle wave amplitude is 1.0 and lowest point is 0.0
    float tri_wave = 
        0.5 + (1.0/M_PI)
         * asin(sin(leaf_get_time_insec()
                     * st->cfg.main.blink_speed));

    term->blink_timer = pow(tri_wave, st->cfg.main.soft_blink_pow);
   
    if(!st->cfg.main.soft_blink) {
        term->blink_timer = floor(term->blink_timer*2);
    }
}

void nmterm_write(NTerminal* term, enum term_write_target target, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[1024 * 4] = { 0 };
    ssize_t len = vsnprintf(buffer, sizeof(buffer)-1, fmt, args);

    if(len > 0) {
        if(target == TERM_WRITE_PTY) {
            write(term->master_fd, buffer, len);
        }
        else
        if(target == TERM_WRITE_VTERM) {
            vterm_input_write(term->vt, buffer, len);
        }
    }

    va_end(args);
}

void nmterm_handle_char_event(Nemi* st, NTerminal* term) {
    if(term == st->messages) {
        return; 
    }

    write(term->master_fd, &st->last_char_in, 1);
    //term->yscroll = 0;
}


void nmterm_handle_key_event(Nemi* st, NTerminal* term) {
    if(term == st->messages) {
        return; 
    }  
    // Im not sure if this is 100% correct
    // but tried to match behaviour with other terminal emulators
    // when looking at 'showkey -a' output.
    if(st->last_key_in >= 'A' && st->last_key_in <= 'Z') {
        int key = -1;

        if(st->last_keymod_in & KEYBOARD_MOD_ALT) {
            key = st->last_key_in + 32;
            write(term->master_fd, "\x1b", 1);
        }

        if(st->last_keymod_in & KEYBOARD_MOD_CONTROL) {
            key = st->last_key_in & 0x1F;
            if(st->last_key_in == KEYBOARD_KEY_L) {
                term->yscroll = 0; // Screen was cleared.
            }
        }

        if(key != -1) {
            write(term->master_fd, &key, 1);
            return;
        }

    }
    else
    if((st->last_key_in == KEYBOARD_KEY_LEFT
     || st->last_key_in == KEYBOARD_KEY_RIGHT)
    && (st->last_keymod_in == KEYBOARD_MOD_SHIFT)) {
        if(st->last_key_in == KEYBOARD_KEY_RIGHT) {
            write(term->master_fd, "\x1b[1;5C", 6);
        }
        if(st->last_key_in == KEYBOARD_KEY_LEFT) {
            write(term->master_fd, "\x1b[1;5D", 6);
        }
        return;
    }

    switch(st->last_key_in) {

        case KEYBOARD_KEY_ENTER:
            term->yscroll = 0;
            nmterm_write(term, TERM_WRITE_PTY, "\r");
            break; 

        case KEYBOARD_KEY_ESCAPE:
            nmterm_write(term, TERM_WRITE_PTY, "\x1b");
            break;

        case KEYBOARD_KEY_TAB:
            nmterm_write(term, TERM_WRITE_PTY, "\x09");
            break;

        case KEYBOARD_KEY_BACKSPACE:
            nmterm_write(term, TERM_WRITE_PTY, "\x08");
            break;

        case KEYBOARD_KEY_UP:
            nmterm_write(term, TERM_WRITE_PTY, "\x1b[A");
            break;

        case KEYBOARD_KEY_DOWN:
            nmterm_write(term, TERM_WRITE_PTY, "\x1b[B");
            break;

        case KEYBOARD_KEY_RIGHT:
            nmterm_write(term, TERM_WRITE_PTY, "\x1b[C");
            break;

        case KEYBOARD_KEY_LEFT:
            nmterm_write(term, TERM_WRITE_PTY, "\x1b[D");
            break;
    }

}

void nmterm_handle_resize_event(Nemi* st, NTerminal* term) {
    //const size_t old_num_cells = term->cols * term->rows;

    term->cols = st->win_cols;
    term->rows = st->win_rows;

    vterm_set_size(term->vt, term->rows, term->cols);

    struct winsize ws = (struct winsize) {
        .ws_row = term->rows,
        .ws_col = term->cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

    ioctl(term->master_fd, TIOCSWINSZ, &ws);

   
    // TODO: We maybedont always need the whole window sized framebuffer.
    leaf_free_framebuffer(&term->cell_framebuffer);
    leaf_free_framebuffer(&term->arbt_framebuffer);
    leaf_create_framebuffer(&term->cell_framebuffer, st->lfctx->win_width, st->lfctx->win_height);
    leaf_create_framebuffer(&term->arbt_framebuffer, st->lfctx->win_width, st->lfctx->win_height);

    /*
    term->hidden_cells = 
        realloc
        (
            term->hidden_cells, 
            (term->cols * term->rows) * sizeof *term->hidden_cells
        );

    size_t num_curr_cells = term->rows * term->cols;

    for(size_t i = old_num_cells; i < num_curr_cells; i++) {
        term->hidden_cells[i] = false;
    }
    */

    // This function will also cause the terminal to be re-rendered.
    resize_nmterm_cell_buffers(term);

    logprintf(LOG_INFO, "Terminal resized. to %ix%i", term->cols, term->rows);
}


// TODO: needed?
void nmterm_handle_altbuffer_change_event(Nemi* st, NTerminal* term) {
    term->yscroll = 0;
}

static inline 
VTermColor get_vtcolor(Nemi* st, int cfgcol_idx) {
    return (VTermColor) {
        .type = VTERM_COLOR_RGB,
        .rgb.type = VTERM_COLOR_RGB,
        .rgb.red    = st->cfg.colors[cfgcol_idx].r,
        .rgb.green  = st->cfg.colors[cfgcol_idx].g,
        .rgb.blue   = st->cfg.colors[cfgcol_idx].b
    };
}

static inline 
void set_term_color(Nemi* st, NTerminal* term, int idx, int cfgcol_idx) {
    VTermColor color = get_vtcolor(st, cfgcol_idx);
    vterm_state_set_palette_color(term->vtstate, idx, &color);
}

void nmterm_init_palette(Nemi* st, NTerminal* term) {
    VTermColor default_fg = get_vtcolor(st, NEMI_COLOR_FG);
    VTermColor default_bg = get_vtcolor(st, NEMI_COLOR_BG);
    vterm_state_set_default_colors(term->vtstate, &default_fg, &default_bg);

    set_term_color(st, term, 0, NEMI_COLOR_BLACK);
    set_term_color(st, term, 1, NEMI_COLOR_RED);
    set_term_color(st, term, 2, NEMI_COLOR_GREEN);
    set_term_color(st, term, 3, NEMI_COLOR_YELLOW);
    set_term_color(st, term, 4, NEMI_COLOR_BLUE);
    set_term_color(st, term, 5, NEMI_COLOR_MAGENTA);
    set_term_color(st, term, 6, NEMI_COLOR_CYAN);
    set_term_color(st, term, 7, NEMI_COLOR_WHITE);
    
    set_term_color(st, term, 8,  NEMI_BRIGHT_COLOR_BLACK);
    set_term_color(st, term, 9,  NEMI_BRIGHT_COLOR_RED);
    set_term_color(st, term, 10, NEMI_BRIGHT_COLOR_GREEN);
    set_term_color(st, term, 11, NEMI_BRIGHT_COLOR_YELLOW);
    set_term_color(st, term, 12, NEMI_BRIGHT_COLOR_BLUE);
    set_term_color(st, term, 13, NEMI_BRIGHT_COLOR_MAGENTA);
    set_term_color(st, term, 14, NEMI_BRIGHT_COLOR_CYAN);
    set_term_color(st, term, 15, NEMI_BRIGHT_COLOR_WHITE);
}

char nmterm_get_char(NTerminal* term, int column, int row) {
    if(column < 0) {
        return 0;        
    }

    // TODO: This can be optimized. 
    // At libvterm side its copying the internal cell to external cell structure.
    VTermScreenCell cell = (VTermScreenCell){ 0 };
    if(!vterm_screen_get_cell(term->vtscrn,  (VTermPos){ row, column }, &cell)) {
        return 0;
    }
    return cell.chars[0];
}


void nmterm_set_cell_custom_bg(NTerminal* term, VTermPos pos, int hex_rgb_color) {
    vterm_screen_set_cell_custom_bg(term->vtscrn, 
            pos,
            (VTermColor) {
                .type = VTERM_COLOR_RGB,
                .rgb.type = VTERM_COLOR_RGB,
                .rgb.red = HEX2RED_CHANNEL(hex_rgb_color),
                .rgb.green = HEX2GRN_CHANNEL(hex_rgb_color),
                .rgb.blue = HEX2BLU_CHANNEL(hex_rgb_color)
            });
}

void nmterm_set_cell_custom_fg(NTerminal* term, VTermPos pos, int hex_rgb_color) {
    vterm_screen_set_cell_custom_fg(term->vtscrn, 
            pos,
            (VTermColor) {
                .type = VTERM_COLOR_RGB,
                .rgb.type = VTERM_COLOR_RGB,
                .rgb.red = HEX2RED_CHANNEL(hex_rgb_color),
                .rgb.green = HEX2GRN_CHANNEL(hex_rgb_color),
                .rgb.blue = HEX2BLU_CHANNEL(hex_rgb_color)
            });
}

void nmterm_set_cell_custom_attrs(NTerminal* term, VTermPos pos, int attrs) {
    vterm_screen_set_cell_custom_attrs(term->vtscrn, pos, attrs);
}

void nmterm_clear_cell_custom_bg(NTerminal* term, VTermPos pos) {
    vterm_screen_clear_cell_custom_bg(term->vtscrn, pos);
}

void nmterm_clear_cell_custom_fg(NTerminal* term, VTermPos pos) {
    vterm_screen_clear_cell_custom_fg(term->vtscrn, pos);
}

void nmterm_clear_cell_custom_attrs(NTerminal* term, VTermPos pos) {
    vterm_screen_clear_cell_custom_attrs(term->vtscrn, pos);
}

/*
void swap_int(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}
*/

/*
void nmterm_copy_to_clipboard(Nemi* st, NTerminal* term, const char* type,
        int start_col, int start_row, int end_col, int end_row) {
    struct string_t buffer = string_create(0);
    end_row++;


    if(STR_MATCH(type, "normal")) {
        if(start_row > end_row) {
            swap_int(&start_row, &end_row);  
            swap_int(&start_col, &end_col);
            start_row--;
            end_row++;
        }

        for(int y = start_row; y < end_row; y++) {
            bool last_row = (y+1 >= end_row);
            int ln_x_stop = last_row ? end_col : term->cols;
            for(int x = start_col; x < ln_x_stop; x++) {
            
                char ch = nmterm_get_char(term, x, y);
                if(ch == 0) {
                    continue;
                }

                string_pushbyte(&buffer, ch);
            }
            string_pushbyte(&buffer, '\n');
            start_col = 0;
        }
    }
    else
    if(STR_MATCH(type, "block")) {
        int sx = MIN_VALUE(start_col, end_col);
        int sy = MIN_VALUE(start_row, end_row);
        int ex = MAX_VALUE(start_col, end_col);
        int ey = MAX_VALUE(start_row, end_row);

        if(start_row > end_row) {
            sy--;
            ey++;
        }

        for(int y = sy; y < ey; y++) {
            for(int x = sx; x < ex; x++) {
                char ch = nmterm_get_char(term, x, y);
                if(ch == 0) {
                    continue;
                }

                string_pushbyte(&buffer, ch);
            }
            string_pushbyte(&buffer, '\n');
        }
    }
    else {
        logprintf(LOG_ERROR, "Unhandled type \"%s\" for copying.", type);
        return;
    }
    
    string_nullterm(&buffer);
    glfwSetClipboardString(st->lfctx->glfw_win, buffer.bytes);

    printf("%s: copied %li bytes to clipboard.\n", __func__, strlen(buffer.bytes));

    free_string(&buffer);
}
*/

void nmterm_yscroll(NTerminal* term, int offset) {
    nmterm_yscroll_to(term, term->yscroll + offset);
}

void nmterm_yscroll_to(NTerminal* term, int point) {
    int old_yscroll = term->yscroll;
    term->yscroll = point;

    nmterm_set_row_dirty(term, term->cursor_row - term->yscroll);

    // We mayb need to clear the rows at the bottom.
    if(old_yscroll < term->yscroll && term->yscroll > 0) {

        //int rows_end_was = term->rows - MAX_VALUE(old_yscroll, 0);
        int rows_end_now = term->rows - MAX_VALUE(term->yscroll, 0);


        Nemi* st = nmt_getst();
        nmterm_clear_screen_row(st, term, rows_end_now);
    }
}


void nmterm_box_move(NTerminal* term, int new_box_x, int new_box_y) {
   
    term->box.x = new_box_x;
    term->box.y = new_box_y;

    for(int i = 0; i < term->rows; i++) {
        nmterm_set_row_dirty(term, i);
    }

}


void nmterm_clear(NTerminal* term) {
    nmterm_write(term, TERM_WRITE_VTERM, "\033[2J\033[H\033[0m");
}

void nmterm_clear_row(NTerminal* term, int row) {
    nmterm_mv_cursor(term, row, 0);
    nmterm_write(term, TERM_WRITE_VTERM, "\033[2K");
}

void nmterm_mv_cursor(NTerminal* term, int row, int column) {
    nmterm_write(term, TERM_WRITE_VTERM, "\033[%i;%iH", row+1, column+1);
}

void nmterm_mv_putchr(NTerminal* term, int row, int column, char chr) {
    nmterm_mv_cursor(term, row, column);
    nmterm_write(term, TERM_WRITE_VTERM, "%c", chr);
}

void nmterm_mv_putstr_nullterm(NTerminal* term, int row, int column, char* str) {
    nmterm_mv_cursor(term, row, column);
    nmterm_write(term, TERM_WRITE_VTERM, "%s", row, column, str);
}

void nmterm_mv_putstrn(NTerminal* term, int row, int column, char* str, size_t len) {
    nmterm_mv_cursor(term, row, column);
    vterm_input_write(term->vt, str, len);
}

void nmterm_clear_row_part(NTerminal* term, int row, int column_begin, int column_end) {
    for(int i = column_begin; i < column_end; i++) {
        nmterm_mv_putchr(term, row, i, ' ');
    }
}




