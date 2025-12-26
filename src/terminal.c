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

    if((int)sbrow->num_cells_alloc != term->cols) {
        sbrow->num_cells_alloc = term->cols;
        size_t num_bytes = sbrow->num_cells_alloc * sizeof *sbrow->cells;
        void* cells_new_ptr = realloc(sbrow->cells, num_bytes);
        if(cells_new_ptr == NULL) {
            logprintf(LOG_ERROR, "Failed to reallocate scrollback buffer row %p cells. "
                    "Tried to allocate %li bytes", sbrow, num_bytes);
            return;
        }
        sbrow->cells = cells_new_ptr;
    }

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

        row->num_cells_alloc = term->cols;
        row->cells = malloc(row->num_cells_alloc * sizeof *row->cells);
        row->num_cells = 0;
    }

    logprintf(LOG_INFO, "Initialized scrollback buffer (%i rows)", SCROLLBACK_LIMIT);
}

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols, const char* shell) {
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
        execlp(shell, shell, NULL);
    }

    term->rows = rows;
    term->cols = cols;

    term->flags = 0;
    term->line_height = st->font.char_height + st->cfg.line_padding;
    term->blink_timer = 0;

    term->hidden_cells = calloc(term->cols * term->rows, sizeof *term->hidden_cells);

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

    terminal_init_palette(st, term);

    logprintf(LOG_INFO, "Created terminal (%ix%i)", term->rows, term->cols);
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
    freeif(term->hidden_cells);

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
            coltox(st, vtcurs_pos.col) * st->cfg.char_spacing,
            rowtoy(st, vtcurs_pos.row + term->sb.offset),
            st->font.char_width,
            st->font.char_height,
            (struct color_t) { 60, 60, 60 });
}

static
struct color_t vtermcolor_to_leafcolor(VTermColor* c) {
    return (struct color_t) {
        c->rgb.red, c->rgb.green, c->rgb.blue
    };
}

static
bool* get_hidden_cell_status(struct terminal* term, int col, int row) {
    size_t index = row * term->cols + col;

    size_t num_cells = term->cols * term->rows;
    if(index >= num_cells) {
        return NULL;
    }

    return &term->hidden_cells[index];
}

void terminal_hide_cells(struct terminal* term, bool hidden, int col, int row, int width, int height) {
    
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


static
void render_cell(struct nemi* st, struct terminal* term, VTermScreenCell* cell, VTermPos pos) {
    
    if(cell->chars[0] == 0) {
        return;
    }

    bool* is_hidden = get_hidden_cell_status(term, pos.col, pos.row);
    if(is_hidden) {
        if(*is_hidden) {
            return;
        }
    }

    int char_x = coltox(st, pos.col);
    int char_y = rowtoy(st, pos.row);

    char_x *= st->cfg.char_spacing;

    // Cell background.

    struct color_t bg_color = st->cfg.colors[NEMI_COLOR_BG];

    if(VTERM_COLOR_IS_INDEXED(&cell->bg)) {
        vterm_state_convert_color_to_rgb(term->vtstate, &cell->bg);
    }

    if(VTERM_COLOR_IS_RGB(&cell->bg)) {
        bg_color = vtermcolor_to_leafcolor(&cell->bg);
        if(bg_color.r != st->cfg.colors[NEMI_COLOR_BG].r
        || bg_color.g != st->cfg.colors[NEMI_COLOR_BG].g
        || bg_color.b != st->cfg.colors[NEMI_COLOR_BG].b) {
            
            leaf_draw_rect(
                    char_x, char_y,
                    st->font.char_width,
                    st->font.char_height,
                    bg_color);
        }
    }

    // Cell foreground.


    struct color_t fg_color = st->cfg.colors[NEMI_COLOR_FG]; // Default color.

    if(VTERM_COLOR_IS_INDEXED(&cell->fg)) {
        vterm_state_convert_color_to_rgb(term->vtstate, &cell->fg);
    }

    if(VTERM_COLOR_IS_RGB(&cell->fg)) {
        fg_color = vtermcolor_to_leafcolor(&cell->fg);    
    }
    
    if(cell->attrs.blink) {
        fg_color = leaf_color_lerp(fg_color, bg_color, term->blink_timer);
    }

    st->font.italic = (cell->attrs.italic) ? st->cfg.italic_tilt : 0.0f;

    leaf_set_font_color(&st->font, fg_color);
    leaf_draw_char(&st->font, char_x, char_y, cell->chars[0]);

    if(cell->attrs.underline) {
        leaf_draw_rect(
                char_x,
                char_y + st->font.char_height + st->cfg.underline_offset,
                st->font.char_width,
                st->cfg.underline_height,
                fg_color);
    }
}


static
void render_scrollback_buffer(struct nemi* st, struct terminal* term) {
    if(term->sb.offset > 0 && term->sb.num_rows > 0) {
        int row = 0;

        ssize_t start_offset = term->sb.num_rows - term->sb.offset;
        if(start_offset < 0) {
            logprintf(LOG_ERROR, "Invalid scrollback offset %li", start_offset);
            return;
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
}

void render_terminal(struct nemi* st, struct terminal* term) {

    vterm_get_size(term->vt, &term->rows, &term->cols);
    if(vterm_screen_is_altscreen(term->vtscrn)) {
        term->sb.offset = 0;
    }

    render_scrollback_buffer(st, term);

    for(int row = 0; row < term->rows; row++) {
        for(int col = 0; col < term->cols; col++) {
            VTermScreenCell cell;
            if(!vterm_screen_get_cell(term->vtscrn,  (VTermPos){ row, col }, &cell)) {
                return;
            }
            render_cell(st, term, &cell,
                    (VTermPos){ row + term->sb.offset, col });
        }
    }

    if(term != st->messages) {
        render_terminal_cursor(st, term);
    }
}

void update_terminal_blink_timer(struct nemi* st, struct terminal* term) {
    if(st->cfg.soft_blink) {
        // https://en.wikipedia.org/wiki/Triangle_wave
        //
        // Here triangle wave amplitude is 1.0 and lowest point is 0.0
        float tri_wave = 0.5+(1.0/M_PI)*asin(sin(glfwGetTime() * st->cfg.blink_speed));

        term->blink_timer = pow(tri_wave, st->cfg.soft_blink_pow);
    }
    else {


    }
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
*/

void terminal_handle_char_event(struct nemi* st, struct terminal* term) {
    if(st->flags & FLG_IGNORE_CHR_INPUT) {
        return;
    }

    if(term == st->messages) {
        return; 
    }
    
    write_term(term, TERM_WRITE_PTY, &st->last_char_in);
    terminal_set_scroll(term, 0);
}


void terminal_handle_key_event(struct nemi* st, struct terminal* term) {
    if(st->flags & FLG_IGNORE_KEY_INPUT) {
        return;
    }
 
    if(term == st->messages) {
        return; 
    }  
    // Im not sure if this is 100% correct
    // but tried to match behaviour with other terminal emulators
    // when looking at 'showkey -a' output.
    if(st->last_key_in >= 'A' && st->last_key_in <= 'Z') {
        int key = -1;

        if(st->last_keymod_in & GLFW_MOD_ALT) {
            key = st->last_key_in + 32;
            write(term->master_fd, "\x1b", 1);
        }

        if(st->last_keymod_in & GLFW_MOD_CONTROL) {
            key = st->last_key_in & 0x1F;
        }

        if(key != -1) { 
            write(term->master_fd, &key, 1);
        }
    }

    switch(st->last_key_in) {

        case GLFW_KEY_ENTER:
            terminal_set_scroll(term, 0);
            write_term(term, TERM_WRITE_PTY, "\r");
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

    int old_rows = term->rows;
    int old_cols = term->cols;

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


    size_t old_num_cells = old_rows * old_cols;

    term->hidden_cells = realloc(term->hidden_cells, 
            (term->cols * term->rows) * sizeof *term->hidden_cells);

    size_t num_curr_cells = term->rows * term->cols;
    for(size_t i = old_num_cells; i < num_curr_cells; i++) {
        term->hidden_cells[i] = false;
    }
}


void terminal_handle_altscreen_change_event(struct nemi* st, struct terminal* term) {
    terminal_set_scroll(term, 0);

    trigger_event_for_scripts(st, REG_EVENT_TERM_BUFFER_CHANGED,
            "i",
            term->is_altscreen);
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


