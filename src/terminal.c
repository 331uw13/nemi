#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pty.h>
#include <poll.h>
#include <stdio.h>
#include <errno.h>

#include "terminal.h"
#include "nemi.h"
#include "common.h"


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

    term->flags = 0;
    term->line_height = st->font.char_height + st->cfg.line_padding;


    term->rows = rows;
    term->cols = cols;
    term->vt = vterm_new(rows, cols);
    vterm_set_utf8(term->vt, true);

    term->vtscrn = vterm_obtain_screen(term->vt);
    term->vtstate = vterm_obtain_state(term->vt);
    vterm_screen_enable_altscreen(term->vtscrn, true);

    terminal_init_palette(st, term);
    vterm_screen_reset(term->vtscrn, true);


    term->vmode.enabled = false;
    term->vmode.curs_row = 0;
    term->vmode.curs_col = 0;
    term->vmode.sel = false;
    term->vmode.sel_start_row = 0;
    term->vmode.sel_start_col = 0;

    return term;
}

void close_terminal(struct terminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }


    close(term->master_fd);
    vterm_free(term->vt);

    term->vt = NULL;
    term->master_fd = -1;
}


void read_terminal(struct nemi* st, struct terminal* term) {
    if(!term) {
        return;
    }

   
    struct pollfd pfd;
    pfd.fd = term->master_fd;
    pfd.events = POLLIN;
    nfds_t num_fds = 1;

    char tmp_buffer[1024*4] = { 0 };

    const int timeout_ms = 10;


    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv > 0) {
           
            memset(tmp_buffer, 0, sizeof(tmp_buffer)-1);
            ssize_t rd = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
            if(rd <= 0) {
                break;
            }

            vterm_input_write(term->vt, tmp_buffer, rd);
    
        }
        else {
            break;
        }
    }
            
    vterm_screen_flush_damage(term->vtscrn);
}


static
void render_terminal_cursor(struct nemi* st, struct terminal* term) {
    VTermPos vtcurs_pos = (VTermPos){ 0, 0 };
    vterm_state_get_cursorpos(term->vtstate, &vtcurs_pos);

    leaf_draw_rect(
            coltox(st, vtcurs_pos.col),
            rowtoy(st, vtcurs_pos.row),
            st->font.char_width,
            st->font.char_height,
            (struct color_t) { 60, 60, 60 }
            );
}

void render_terminal(struct nemi* st, struct terminal* term) {

    vterm_get_size(term->vt, &term->rows, &term->cols);

    for(int row = 0; row < term->rows; row++) {
        for(int col = 0; col < term->cols; col++) {

            VTermScreenCell cell;
            if(!vterm_screen_get_cell(term->vtscrn, (VTermPos){ row, col }, &cell)) {
                continue;
            }

            if(cell.chars[0] == 0) {
                continue;
            }

            if(VTERM_COLOR_IS_INDEXED(&cell.fg)) {
                vterm_state_convert_color_to_rgb(term->vtstate, &cell.fg);
            }

            if(VTERM_COLOR_IS_RGB(&cell.fg)) {
                leaf_set_font_color(&st->font,
                        (float)cell.fg.rgb.red   / 255.0f,
                        (float)cell.fg.rgb.green / 255.0f,
                        (float)cell.fg.rgb.blue  / 255.0f);
            }

            leaf_draw_char(&st->font, 
                    coltox(st, col),
                    rowtoy(st, row), cell.chars[0]);
        }
    }

    render_terminal_cursor(st, term);
}

void write_terminal(struct terminal* term, char* buffer, size_t size) {
    if(term->master_fd < 0) {
        return;
    }

    write(term->master_fd, buffer, size);
}

void terminal_handle_char_event(struct nemi* st, struct terminal* term) {
    
    write_terminal(term, &st->last_char_in, 1);

}


void terminal_handle_key_event(struct nemi* st, struct terminal* term) {
    
    if(key_down(st, GLFW_KEY_LEFT_CONTROL)
    || key_down(st, GLFW_KEY_RIGHT_CONTROL)) {
        
        switch(st->last_key_in) {
            
            case GLFW_KEY_L:
                write_terminal(term, "clear\n", 6);
                break;

            case GLFW_KEY_C:
                write_terminal(term, "\03", 1);
                break;

            case GLFW_KEY_M:
                term->vmode.enabled = !term->vmode.enabled;
                break;
        }

        return;
    }


    switch(st->last_key_in) {

        case GLFW_KEY_ENTER:
            write_terminal(term, "\n", 1);
            break; 

        case GLFW_KEY_ESCAPE:
            write_terminal(term, "\x1b", 1);
            break;


        case GLFW_KEY_TAB:
            write_terminal(term, "\x09", 1);
            break;

        case GLFW_KEY_BACKSPACE:
            write_terminal(term, "\x08", 1);
            break;

        case GLFW_KEY_UP:
            write_terminal(term, "\x1b[A", 3);
            break;

        case GLFW_KEY_DOWN:
            write_terminal(term, "\x1b[B", 3);
            break;

        case GLFW_KEY_RIGHT:
            write_terminal(term, "\x1b[C", 3);
            break;

        case GLFW_KEY_LEFT:
            write_terminal(term, "\x1b[D", 3);
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







