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

    term->is_altscreen = false;

    /*
    term->vmode.mode = VMODE_MODE_FILES;
    term->vmode.enabled = false;
    term->vmode.curs.row = 0;
    term->vmode.curs.col = 0;
    term->vmode.sel_start_row = 0;
    term->vmode.sel_start_col = 0;
    */
    terminal_init_palette(st, term);

    // Add vmode word separators.
    //terminal_vmode_add_word_sep(term, ' ');
    //terminal_vmode_add_word_sep(term, '\0');

    logprintf(LOG_INFO, "Created terminal (%ix%i)", term->rows, term->cols);
    //printf("%s() %p (%ix%i)\n", __func__, term, term->rows, term->cols);

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

    char tmp_buffer[1024*2] = { 0 };

    const int timeout_ms = 10;


    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv <= 0) {
            break;
          
        }
        
        memset(tmp_buffer, 0, sizeof(tmp_buffer));
        ssize_t rd = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
        if(rd <= 0) {
            break;
        }

        for(ssize_t i = 0; i < rd; i++) {
            vterm_input_write(term->vt, &tmp_buffer[i], 1);
            vterm_screen_flush_damage(term->vtscrn);
        }
    }


    bool now_altscreen = vterm_screen_is_altscreen(term->vtscrn);
    if(now_altscreen != term->is_altscreen) {
        term->is_altscreen = now_altscreen;
        terminal_handle_altscreen_change_event(st, term);
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
    
    int char_x = coltox(st, pos.col);
    int char_y = rowtoy(st, pos.row);

    st->font.italic = (cell->attrs.italic) ? st->cfg.italic_tilt : 0.0f;

    leaf_draw_char(&st->font, char_x, char_y, cell->chars[0]);

    if(cell->attrs.underline) {
        leaf_draw_rect(
                char_x,
                char_y + st->font.char_height + st->cfg.underline_offset,
                st->font.char_width,
                st->cfg.underline_height,
                (struct color_t){ 
                    cell->fg.rgb.red, 
                    cell->fg.rgb.green, 
                    cell->fg.rgb.blue, 
                });
    }
}
/*
static
void render_vmode(struct nemi* st, struct terminal* term) {
        
    leaf_set_font_color(&st->font, 0.7, 0.3, 0.5);

    int label_width = coltox(st, 10);

    leaf_draw_text_fmt(&st->font, st->lfctx->win_width - label_width, 10, 
            "[vmode]:%c", (char)term->vmode.mode);
}
*/

void render_terminal(struct nemi* st, struct terminal* term) {

    vterm_get_size(term->vt, &term->rows, &term->cols);
    if(vterm_screen_is_altscreen(term->vtscrn)) {
        term->sb.offset = 0;
    }

    if(term->sb.offset > 0 && term->sb.num_rows > 0) {
        int row = 0;

        ssize_t start_offset = term->sb.num_rows - term->sb.offset;
        if(start_offset < 0) {
            logprintf(LOG_ERROR, "Invalid scrollback offset %li", start_offset);
            goto scrollback_err;
        }
            
        struct scrollback_row* sbrow = &term->sb.rows[ start_offset ];

        while(row < term->sb.offset) {
            for(int col = 0; col < term->cols; col++) {
                render_cell(st, term, &sbrow->cells[col], 
                        (VTermPos){ row, col });
            }
            row++;
            sbrow++;
        }
    }
scrollback_err:

    for(int row = 0; row < term->rows; row++) {
        for(int col = 0; col < term->cols; col++) {
            VTermScreenCell cell;
            if(!vterm_screen_get_cell(term->vtscrn, 
                        (VTermPos){ row, col }, &cell)) {
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

    char buffer[320] = { 0 };
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


/*
// TODO: 
// This function takes in count the scrollback buffer
static
bool terminal_get_char(struct terminal* term, char* ch, VTermPos pos) {
    VTermScreenCell cell;
    if(!vterm_screen_get_cell(term->vtscrn, (VTermPos){ pos.row, pos.col }, &cell)) {
        return false;
    }

    *ch = cell.chars[0];

    return true;
}


static
bool vmode_is_separator(struct terminal* term, char ch) {
    for(uint32_t i = 0; i < term->vmode.num_word_separators; i++) {
        if(ch == term->vmode.word_separators[i]) {
            return true;
        }
    }
    return false;
}

static
void terminal_vmode_read_word(struct terminal* term) {

    memset(term->vmode.word, 0, sizeof(term->vmode.word));
    term->vmode.word_len = 0;
   


    // Read the whole row first.
    char row_chars [term->cols];
    uint32_t row_len = 0;
    memset(row_chars, 0, sizeof(row_chars));

    for(int i = 0; i < term->cols; i++) {
        if(terminal_get_char(term, &row_chars[row_len], 
                    (VTermPos){ term->vmode.curs.row, i })) {
            if(row_chars[row_len] == 0) {
                row_len++;
                break;
            }
            row_len++;
        };
    }


    int word_start_col = -1;
    int word_end_col   = -1;

    bool curs_on_word = !vmode_is_separator(term, row_chars[ term->vmode.curs.col ]);

    if(curs_on_word) {
        // Get start column by going left and searching for word separator.
        for(int c = term->vmode.curs.col; c > 0; c--) {
            if(vmode_is_separator(term, row_chars[c])) {
                word_start_col = c + 1;
                break;
            }
        }
        if(word_start_col < 0) {
            word_start_col = 0;
        }

        // Get end column by going right and searching for word separator.
        for(int c = term->vmode.curs.col; c < term->cols; c++) {
            if(vmode_is_separator(term, row_chars[c])) {
                word_end_col = c;
                break;
            }
        }

        if(word_start_col >= 0 && word_end_col >= 0) {

            for(int c = word_start_col; c < word_end_col; c++) {
                term->vmode.word[term->vmode.word_len++] = row_chars[c];
            }

        }
    }
}
*/
/*
void terminal_move_vmode(struct terminal* term, int col_off, int row_off, enum vmode_move_opt mov_opt) {

    if(mov_opt == VMODE_MOVE_CELL) {

        term->vmode.curs.row += row_off;
        term->vmode.curs.col += col_off;

    }
    else
    if(mov_opt == VMODE_MOVE_WORD) {
        printf("VMODE_MOVE_WORD - Not implemented yet.\n");
    }

    terminal_vmode_read_word(term);
}

void terminal_vmode_add_word_sep(struct terminal* term, char ch) {
    if(term->vmode.num_word_separators+1 >= ARRAY_LEN(term->vmode.word_separators)) {
        return;
    }

    term->vmode.word_separators[ 
        term->vmode.num_word_separators++
    ] = ch;
}

void terminal_vmode_file_interact(struct terminal* term) {
   
    if(term->vmode.word_len == 0) {
        return;
    }

    printf("Interact file '%s'\n", term->vmode.word);

    if(access(term->vmode.word, F_OK) != 0) {
        printf("%s: access: %s\n", __func__, strerror(errno));
        return;
    }

    struct stat sb;
    if(stat(term->vmode.word, &sb) != 0) {
        printf("%s: stat: %s\n", __func__, strerror(errno));
        return;
    }
    

    switch(sb.st_mode & S_IFMT) {
    
        case S_IFDIR:
            break;

        case S_IFREG:
            break;


        default:
            printf("\033[33m%s: Unhandled file type.\033[0m\n", __func__);
            break;


    }

}

static
void terminal_handle_vmode_input(struct nemi* st, struct terminal* term) {

    switch(st->last_key_in) {

        case GLFW_KEY_I: // Up
            terminal_move_vmode(term, 0, -1, VMODE_MOVE_CELL);
            break;

        case GLFW_KEY_K: // Down
            terminal_move_vmode(term, 0, 1, VMODE_MOVE_CELL);
            break;

        case GLFW_KEY_L: // Right
            terminal_move_vmode(term, 1, 0, VMODE_MOVE_CELL);
            break;

        case GLFW_KEY_J: // Left
            terminal_move_vmode(term, -1, 0, VMODE_MOVE_CELL);
            break;

        case GLFW_KEY_F:
            term->vmode.mode = VMODE_MODE_FILES;
            break;

        case GLFW_KEY_S:
            term->vmode.mode = VMODE_MODE_SEL;
            break;

        case GLFW_KEY_V:
            term->vmode.mode = VMODE_MODE_BLOCK_SEL;
            break;
 
        case GLFW_KEY_R:
        case GLFW_KEY_ENTER:
            if(term->vmode.mode == VMODE_MODE_FILES) {
                terminal_vmode_file_interact(term);
            }
            break;
    }
}
*/

void terminal_handle_char_event(struct nemi* st, struct terminal* term) {

    if(st->last_char_in != 0) {
        char* args[] = {
            &st->last_char_in,
            NULL
        };

        for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
            struct perl_script* script = &st->scripts[i];
            if(!script->is_loaded) {
                continue;
            }
            plscript_call_args(script, "event_char_input", args);
        }
    }

    if(st->flags & FLG_IGNORE_CHR_INPUT) {
        return;
    }
    
    write_term(term, TERM_WRITE_PTY, &st->last_char_in);
    terminal_set_scroll(term, 0);
}


void terminal_handle_key_event(struct nemi* st, struct terminal* term) {
 
    if(st->last_key_in != 0) {
        char key_str[8] = { 0 };
        snprintf(key_str, sizeof(key_str), "%d", st->last_key_in);
        char* args[] = {
            key_str,
            NULL
        };

        for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
            struct perl_script* script = &st->scripts[i];
            if(!script->is_loaded) {
                continue;
            }
            plscript_call_args(script, "event_key_input", args);
        }
    }   

    if(st->flags & FLG_IGNORE_KEY_INPUT) {
        return;
    }

    if(key_down(st, GLFW_KEY_LEFT_CONTROL)
    || key_down(st, GLFW_KEY_RIGHT_CONTROL)) {
        
        switch(st->last_key_in) { 

            case GLFW_KEY_C:
                write_term(term, TERM_WRITE_PTY, "\03");
                break;

            case GLFW_KEY_E:
                write_term(term, TERM_WRITE_PTY, "clear\n");
                break;
                
            case GLFW_KEY_I:
                terminal_scroll(term, +1);
                break;

            case GLFW_KEY_K:
                terminal_scroll(term, -1);
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


void terminal_handle_altscreen_change_event(struct nemi* st, struct terminal* term) {
    
    /*if(term->is_altscreen) {
        term->vmode.was_enabled_before_altscreen = term->vmode.enabled;
        term->vmode.enabled = false;
    }
    else {
        term->vmode.enabled = term->vmode.was_enabled_before_altscreen;
    }*/

    terminal_set_scroll(term, 0);
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


