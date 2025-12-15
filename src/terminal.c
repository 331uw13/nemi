#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <pty.h>
#include <poll.h>
#include <stdio.h>
#include <errno.h>

#include "terminal.h"
#include "nemi.h"
#include "common.h"
#include "memory.h"


void vterm_scroll_callback(VTermRect rect, int downward, int rightward, void *userptr) {

    if(downward < 0) {
        return;
    }

    struct terminal* term = (struct terminal*)userptr;

    if(vterm_screen_is_altscreen(term->vtscrn)) {
        return;
    }

    const int cell_row = 0;
   
    struct scrollback_row* sbrow = &term->sb.rows[ term->sb.num_rows ];
    if(term->sb.num_rows >= term->sb.num_rows_max) {
        printf("\033[31mTODO: Shift scrollback buffer.\033[0m\n");
        return;
    }
    else {
        term->sb.num_rows++;
    }

    sbrow->num_cells = 0;

    for(int col = 0; col < term->cols; col++) {
        if(vterm_screen_get_cell(term->vtscrn, 
                (VTermPos){ cell_row, col }, &sbrow->cells[ sbrow->num_cells ])) {
            sbrow->num_cells++;
        }
    }
}



#define SCROLLBACK_LIMIT  1000  // TODO: Add to config.
static
void init_scrollback_buffer(struct terminal* term) {

    term->sb.offset = 0;
    term->sb.num_rows = 0;
    term->sb.num_rows_max = SCROLLBACK_LIMIT;
    term->sb.rows = malloc(SCROLLBACK_LIMIT * sizeof *term->sb.rows);
    for(size_t i = 0; i < SCROLLBACK_LIMIT; i++) {
        struct scrollback_row* row = &term->sb.rows[i];

        row->cells = malloc(term->cols * sizeof *row->cells);
        row->num_cells = 0;
    }

}

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols) {
    if(st->num_terminals+1 >= NEMI_TERMINALS_MAX) {
        return NULL;
    }
    
    struct winsize ws = (struct winsize) {
        .ws_row = rows,
        .ws_col = cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

    struct terminal* term = &st->terminals[st->num_terminals++];
    term->pid = forkpty(&term->master_fd, NULL, NULL, &ws);

    if(term->pid == 0) {
        const char* shell = getenv("SHELL");
        execlp(shell, shell, NULL);
    }

    term->rows = rows;
    term->cols = cols;

    term->flags = 0;
    term->line_height = st->font.char_height + st->cfg.line_padding;


    
    init_scrollback_buffer(term);
    term->vt = vterm_new(rows, cols);
    vterm_set_utf8(term->vt, true);
    

    
    term->vtscrn = vterm_obtain_screen(term->vt);
    term->vtstate = vterm_obtain_state(term->vt);

    //vterm_screen_set_putglyph_callback(term->vtscrn, vterm_putglyph_callback, term);
    vterm_state_set_scroll_callback(term->vtstate, vterm_scroll_callback, term);
            
    vterm_screen_enable_altscreen(term->vtscrn, true);
    vterm_screen_reset(term->vtscrn, true);



    term->vmode.enabled = false;
    term->vmode.curs_row = 0;
    term->vmode.curs_col = 0;
    term->vmode.sel = false;
    term->vmode.sel_start_row = 0;
    term->vmode.sel_start_col = 0;

    terminal_init_palette(st, term);

    printf("%s() %p (%ix%i)\n", __func__, term, term->rows, term->cols);

    return term;
}

void close_terminal(struct terminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }

    for(size_t i = 0; i < term->sb.num_rows_max; i++) {
        freeif(term->sb.rows[i].cells);
    }
    freeif(term->sb.rows);


    close(term->master_fd);
    term->master_fd = -1;
    
    vterm_free(term->vt);
    term->vt = NULL;
}


void read_terminal(struct nemi* st, struct terminal* term) {
    if(!term) {
        return;
    }

   
    struct pollfd pfd;
    pfd.fd = term->master_fd;
    pfd.events = POLLIN;
    nfds_t num_fds = 1;

    char tmp_buffer[32] = { 0 };

    const int timeout_ms = 10;


    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv > 0) {
           
            memset(tmp_buffer, 0, sizeof(tmp_buffer));
            ssize_t rd = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
            if(rd <= 0) {
                break;
            }

            for(size_t i = 0; i < rd; i++) {
                vterm_input_write(term->vt, &tmp_buffer[i], 1);
                vterm_screen_flush_damage(term->vtscrn);
            }
        }
        else {
            break;
        }
    }            
}


static
void render_terminal_cursor(struct nemi* st, struct terminal* term) {
    VTermPos vtcurs_pos = (VTermPos){ 0, 0 };
    vterm_state_get_cursorpos(term->vtstate, &vtcurs_pos);

    leaf_draw_rect(
            coltox(st, vtcurs_pos.col),
            rowtoy(st, vtcurs_pos.row + term->sb.offset),
            st->font.char_width,
            st->font.char_height,
            (struct color_t) { 60, 60, 60 });
}


static
void render_cell(struct nemi* st, struct terminal* term, VTermScreenCell* cell, VTermPos pos) {
    
    if(cell->chars[0] == 0) {
        return;
    }

    if(VTERM_COLOR_IS_INDEXED(&cell->fg)) {
        vterm_state_convert_color_to_rgb(term->vtstate, &cell->fg);
    }

    if(VTERM_COLOR_IS_RGB(&cell->fg)) {
        leaf_set_font_color(&st->font,
                (float)cell->fg.rgb.red   / 255.0f,
                (float)cell->fg.rgb.green / 255.0f,
                (float)cell->fg.rgb.blue  / 255.0f);
    }

    leaf_draw_char(&st->font, 
            coltox(st, pos.col),
            rowtoy(st, pos.row), cell->chars[0]);
}

void render_terminal(struct nemi* st, struct terminal* term) {

    vterm_get_size(term->vt, &term->rows, &term->cols);

    if(vterm_screen_is_altscreen(term->vtscrn)) {
        term->sb.offset = 0;
    }

    if(term->sb.offset > 0 && term->sb.num_rows > 0) {
        int row = 0;
        int offset = 1;

        ssize_t start_offset = term->sb.num_rows - term->sb.offset;
        if(start_offset < 0) {
            fprintf(stderr, "Invalid scrollback offset.\n");
            goto scrollback_err;
        }
            
        struct scrollback_row* sbrow = &term->sb.rows[ start_offset ];

        while(row < term->sb.offset) {
            for(int col = 0; col < term->cols; col++) {
                render_cell(st, term, &sbrow->cells[col], (VTermPos){ row, col });
            }
            row++;
            sbrow++;
        }
    }
scrollback_err:

    for(int row = 0; row < term->rows; row++) {
        for(int col = 0; col < term->cols; col++) {
            VTermScreenCell cell;
            if(!vterm_screen_get_cell(term->vtscrn, (VTermPos){ row, col }, &cell)) {
                return;
            }
            render_cell(st, term, &cell,
                    (VTermPos){ row + term->sb.offset, col });
        }
    }

    render_terminal_cursor(st, term);
}

void write_term(struct terminal* term, enum term_write_target target, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[512] = { 0 };
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

void terminal_scroll(struct terminal* term, int offset) {
    terminal_set_scroll(term, term->sb.offset + offset);
}

void terminal_set_scroll(struct terminal* term, int scroll) {
    if(vterm_screen_is_altscreen(term->vtscrn)) {
        term->sb.offset = 0;
        return;
    }

    term->sb.offset = scroll;
    term->sb.offset = clampi(term->sb.offset, 0, term->sb.num_rows);
}

void terminal_handle_char_event(struct nemi* st, struct terminal* term) {

    write_term(term, TERM_WRITE_PTY, &st->last_char_in);

}



void terminal_handle_key_event(struct nemi* st, struct terminal* term) {
    
    if(key_down(st, GLFW_KEY_LEFT_CONTROL)
    || key_down(st, GLFW_KEY_RIGHT_CONTROL)) {
        
        switch(st->last_key_in) { 

            case GLFW_KEY_C:
                write_term(term, TERM_WRITE_PTY, "\03");
                break;

            case GLFW_KEY_E:
                write_term(term, TERM_WRITE_PTY, "clear\n");
                break;
                
            case GLFW_KEY_O:
                terminal_scroll(term, +1);
                break;

            case GLFW_KEY_L:
                terminal_scroll(term, -1);
                break;

            case GLFW_KEY_M:
                term->vmode.enabled = !term->vmode.enabled;
                break;
        }

        return;
    }


    switch(st->last_key_in) {

        case GLFW_KEY_ENTER:
            terminal_set_scroll(term, 0);
            write_term(term, TERM_WRITE_PTY, "\n");
            break; 

        case GLFW_KEY_ESCAPE:
            write_term(term, TERM_WRITE_PTY, "\x1b");
            break;

        case GLFW_KEY_TAB:
            write_term(term, TERM_WRITE_PTY, "\x09");
            break;

        case GLFW_KEY_BACKSPACE:
            write_term(term, TERM_WRITE_PTY, "\x08");
            break;

        case GLFW_KEY_UP:
            write_term(term, TERM_WRITE_PTY, "\x1b[A");
            break;

        case GLFW_KEY_DOWN:
            write_term(term, TERM_WRITE_PTY, "\x1b[B");
            break;

        case GLFW_KEY_RIGHT:
            write_term(term, TERM_WRITE_PTY, "\x1b[C");
            break;

        case GLFW_KEY_LEFT:
            write_term(term, TERM_WRITE_PTY, "\x1b[D");
            break;

    }
}

void terminal_handle_resize_event(struct nemi* st, struct terminal* term) {

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
}


static
VTermColor get_vtcolor(struct nemi* st, int cfgcol_idx) {
    return (VTermColor) {
        .type = VTERM_COLOR_RGB,
        .rgb.type = VTERM_COLOR_RGB,
        .rgb.red    = st->cfg.colors[cfgcol_idx].r,
        .rgb.green  = st->cfg.colors[cfgcol_idx].g,
        .rgb.blue   = st->cfg.colors[cfgcol_idx].b
    };
}

static
void set_term_color(struct nemi* st, struct terminal* term, int idx, int cfgcol_idx) {
    VTermColor color = get_vtcolor(st, cfgcol_idx);
    vterm_state_set_palette_color(term->vtstate, idx, &color);
}

void terminal_init_palette(struct nemi* st, struct terminal* term) {
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


