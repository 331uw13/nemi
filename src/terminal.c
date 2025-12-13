#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pty.h>
#include <poll.h>
#include <stdio.h>

#include "terminal.h"
#include "nemi.h"
#include "common.h"


struct terminal* spawn_terminal(struct nemi* st) {
    if(st->num_terminals+1 >= NEMI_TERMINALS_MAX) {
        return NULL;
    }

    struct terminal* term = &st->terminals[st->num_terminals++];
    term->pid = forkpty(&term->master_fd, NULL, NULL, NULL);

    if(term->pid == 0) {
        const char* shell = getenv("SHELL");
        execlp(shell, shell, NULL);
    }

    term->lines = NULL;
    term->num_lines = 0;
    term->num_lines_alloc = 0;
    term->flags = 0;
    term->scroll = (struct vec2i){ 0, 0 };   
    term->cursor.pos = (struct vec2i){ 0, 0 };
    term->line_height = st->font.char_height + st->cfg.line_padding_y;

    terminal_handle_resize_event(st, term);

    memset(term->title, 0, sizeof(term->title));
    return term;
}

void close_terminal(struct terminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }

    for(size_t i = 0; i < term->num_lines; i++) {
        free_tline(&term->lines[i]);
    }
    freeif(term->lines);
    term->lines = NULL;
    term->num_lines = 0;
    term->num_lines_alloc = 0;

    close(term->master_fd);
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
    size_t out_index = 0;

    const int timeout_ms = 10;
    bool last_flushed = false;

    size_t old_num_lines = term->num_lines;

    // Assume last line ends with LF ('\n')
    //term->flags |= TERMFLG_LASTLN_HAS_LF;

    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv > 0) {
           
            memset(tmp_buffer, 0, sizeof(tmp_buffer)-1);
            ssize_t rd = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
            if(rd <= 0) {
                break;
            }

            size_t offset = 0;

            if((term->flags & NO_INPUT_ECHO)) {
                term->flags &= ~NO_INPUT_ECHO;
                char* ch = &tmp_buffer[0];
                while(ch < tmp_buffer + rd) {
                    ch++;
                    offset++;
                    if(*ch == '\n') {
                        offset++;
                        break;
                    }
                }
                rd -= offset;
                if(rd <= 0) {
                    continue;
                }
            }

            print_literal(tmp_buffer + offset, rd);
            terminal_add_chars(st, term, tmp_buffer+offset, rd);
        }
        else {
            break;
        }
    }

    /*
    if(!(term->flags & TERMFLG_LASTLN_HAS_LF)) {
        //term->num_lines++;
    }
    */

    if(old_num_lines != term->num_lines) {
        terminal_handle_data_event(st, term);
    }
}

bool term_prep_line_add(struct terminal* term, const char* caller_func) {
    if(!term) {
        return false;
    }
    if(term->master_fd < 0) {
        return false;
    }

    if(term->num_lines + 1 < term->num_lines_alloc) {
        return true;
    }

    const size_t num_alloc_more = 100;
    const size_t new_num_alloc = term->num_lines_alloc + num_alloc_more;

    struct tline* new_ptr = realloc(term->lines, new_num_alloc * sizeof *term->lines);
    if(!new_ptr) {
        // TODO: Create better error handling system, This sucks.
        fprintf(stderr, "Some terminal experienced memory error. %s() Called from %s()\n",
                __func__, caller_func);
        return false;
    }

    term->lines = new_ptr;

    // Initialize new lines.
    for(size_t i = term->num_lines_alloc; i < new_num_alloc; i++) {
        term->lines[i] = create_tline();
    }

    term->num_lines_alloc = new_num_alloc;

    return true;
}

void terminal_add_chars(struct nemi* st, struct terminal* term, char* buffer, size_t size) {
    if(!term_prep_line_add(term, __func__)) {
        return;
    }

    tline_add_buf_to_currln(st, term, buffer, size);
}

void execute_cmd(struct terminal* term, char* cmd_str, size_t cmd_len) {
    if(term->master_fd < 0) {
        return;
    }
    if(cmd_len == 0) {
        return;
    }

    // Dont echo the command which was executed.
    term->flags |= NO_INPUT_ECHO; 
    
    write(term->master_fd, cmd_str, cmd_len);
    if(cmd_str[cmd_len-1] != '\n') {
        write(term->master_fd, "\n", 1);
    }
}

void move_cursor_to_prompt(struct terminal* term) {
    struct tline* last_line = get_terminal_lastln(term);
    if(!last_line) {
        return;
    }

    term->cursor.pos.x = last_line->num_chars;
    term->cursor.pos.y = term->num_lines;
}

static void render_terminal_cursor(struct nemi* st, struct terminal* term) {

    int cursor_drw_x = term->cursor.pos.x;
    int cursor_drw_y = term->cursor.pos.y;

    to_grid_pos(st, &cursor_drw_x, &cursor_drw_y);

    leaf_draw_rect(
            cursor_drw_x,
            cursor_drw_y + term->scroll.y,
            st->font.char_width,
            st->font.char_height,
            (struct color_t){ 50, 80, 80 });
}

void render_terminal(struct nemi* st, struct terminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }
    if(!term->lines) {
        return;
    }


    int yoffset = 10;

    for(size_t i = 0; i < term->num_lines+1; i++) {
        struct tline* line = &term->lines[i];
        
        tline_render(st, term, line, yoffset);
        yoffset += term->line_height;
    }
   
    render_terminal_cursor(st, term);
}

void scroll_terminal_down(struct nemi* st, struct terminal* term) {
    //int end = get_terminal_num_newlines(term) * term->line_height;
    //term->scroll.y = -end + ((term->rows-1) * term->line_height - st->cfg.rows_end_padding);

    int end = term->num_lines * term->line_height;
    term->scroll.y = -end + ((term->rows - 1) * term->line_height - st->cfg.rows_end_padding);
}

void scroll_terminal(struct nemi* st, struct terminal* term, struct vec2i offset) {
    term->scroll.x += offset.x * st->cfg.scroll_x_mult;
    term->scroll.y += offset.y * st->cfg.scroll_y_mult;
}

bool terminal_onlastpage(struct terminal* term) {
    //return (term->scroll.y + term->rows * term->line_height >= term->num_lines);

    int scroll = term->scroll.y / term->line_height;
    return (-scroll > term->num_lines - term->rows);
}

void set_terminal_title(struct terminal* term, char* buffer, size_t len) {
    memset(term->title, 0, sizeof(term->title));
    const size_t max_len = sizeof(term->title) - 4;
    const size_t safe_len = (len < max_len) ? len : max_len;

    if(len > sizeof(term->title)) {
        term->title[safe_len]   = '.';
        term->title[safe_len+1] = '.';
        term->title[safe_len+2] = '.';
    }
    memmove(term->title, buffer, safe_len);
}

struct tline*  get_terminal_lastln(struct terminal* term) {
    struct tline* line = NULL;

    if(!term) {
        goto out;
    }
    if(term->master_fd < 0) {
        goto out;
    }
    if(!term->lines) {
        goto out;
    }

    line = &term->lines[term->num_lines];

out:
    return line;
}

void terminal_clear(struct terminal* term) {
    for(size_t i = 0; i < term->num_lines+1; i++) {
        memset(&term->lines[i], 0, sizeof(*term->lines));
    }
    term->num_lines = 0;
    term->scroll.y = 0;
    term->scroll.x = 0;
    term->currln = &term->lines[0];
    write(term->master_fd, "\n", 1);
}

void terminal_handle_resize_event(struct nemi* st, struct terminal* term) {
    // Temporary.
    term->rows
        = st->lfctx->win_height / term->line_height;

    term->cols 
        = st->lfctx->win_width / st->font.char_width;
}

void terminal_handle_char_event(struct nemi* st, struct terminal* term) {
    if(st->last_char_in == 0) {
        return;
    }

    struct tline* curr_line = get_terminal_lastln(term);
    tline_add_buf_to_currln(st, term, &st->last_char_in, 1);

    term->flags |= NO_INPUT_ECHO;
    write(term->master_fd, &st->last_char_in, 1);
}

void terminal_handle_key_event(struct nemi* st, struct terminal* term) {
    switch(st->last_key_in) {

        case GLFW_KEY_UP:
            write(term->master_fd, "\x1b[A", 3);
            break;

        case GLFW_KEY_DOWN:
            write(term->master_fd, "\x1b[B", 3);
            break;

        case GLFW_KEY_LEFT:
            write(term->master_fd, "\x1b[D", 3);
            break;

        case GLFW_KEY_RIGHT:
            write(term->master_fd, "\x1b[C", 3);
            break;

        case GLFW_KEY_ENTER:
            write(term->master_fd, "\n", 1);
            break;
    }

    if(key_down(st, GLFW_KEY_LEFT_CONTROL)) {
        if(key_down(st, GLFW_KEY_C)) {
            write(term->master_fd, "\3", 1);
        }
        else
        if(key_down(st, GLFW_KEY_L)) {
            terminal_clear(term);
        }
    }
}

void terminal_handle_data_event(struct nemi* st, struct terminal* term) {    
    if(term->num_lines > term->rows) {
        scroll_terminal_down(st, st->terminal);
    }
}

