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
        execlp(getenv("SHELL"), NULL, NULL);
    }

    term->lines = NULL;
    term->num_lines = 0;
    term->num_lines_alloc = 0;
    term->cursor.pos.x = 0;
    term->cursor.pos.y = 0;
    term->flags = 0;

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


void read_terminal(struct terminal* term, size_t* read_bytes, char* out, size_t mem_size) {
    if(!read_bytes) {
        return;
    }
    if(!term) {
        return;
    }
    if(!out || !mem_size) { 
        return;
    }

        
    *read_bytes = 0;

    struct pollfd pfd;
    pfd.fd = term->master_fd;
    pfd.events = POLLIN;
    nfds_t num_fds = 1;

    char tmp_buffer[1024] = { 0 };
    size_t out_index = 0;

    const int timeout_ms = 10;

    while(true) {
        int retv = poll(&pfd, num_fds, timeout_ms);
        if(retv > 0) {
           
            ssize_t rd = read(term->master_fd, tmp_buffer, sizeof(tmp_buffer)-1);
            if(rd <= 0) {
                break;
            }

            if((term->flags & TERMFLG_DROP_NEXT_READ)) {
                term->flags &= ~TERMFLG_DROP_NEXT_READ;
                continue;
            }

            memcpy(out + out_index, tmp_buffer, rd);
            out_index += rd;

            *read_bytes += (size_t)rd;
        }
        else {
            break;
        }
    }
}

static bool term_prep_line_add(struct terminal* term, const char* caller_func) {
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

void push_terminal_line(struct nemi* st, struct terminal* term, char* line_str, size_t line_len) {
    if(!term_prep_line_add(term, __func__)) {
        return;
    }

    struct tline* line = &term->lines[term->num_lines++];
    tline_add(st, line, line_str, line_len);
}

void execute_cmd(struct terminal* term, char* cmd_str, size_t cmd_len) {
    if(term->master_fd < 0) {
        return;
    }
    if(cmd_len == 0) {
        return;
    }

    // Dont echo the command which was executed.
    term->flags |= TERMFLG_DROP_NEXT_READ; 
    
    write(term->master_fd, cmd_str, cmd_len);
    if(cmd_str[cmd_len-1] != '\n') {
        write(term->master_fd, "\n", 1);
    }
}

void move_cursor_to_end(struct terminal* term) {
    
    struct tline* last_line = &term->lines[term->num_lines > 0 ? term->num_lines-1 : 0];

    term->cursor.pos.y = term->num_lines - 1;
    term->cursor.pos.x = last_line->num_chars;

}

static void render_terminal_cursor(struct nemi* st, struct terminal* term) {

    int cursor_drw_x = term->cursor.pos.x;
    int cursor_drw_y = term->cursor.pos.y;

    to_grid_pos(st, &cursor_drw_x, &cursor_drw_y);

    leaf_draw_rect(
            cursor_drw_x, cursor_drw_y,
            st->font.char_width,
            st->font.char_height,
            (struct color_t){ 50, 80, 80 }
            );
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

    render_terminal_cursor(st, term);

    int yoffset = 10;

    for(size_t i = 0; i < term->num_lines; i++) {
        struct tline* line = &term->lines[i];

        struct vec2i line_pos = (struct vec2i) {
            10,
            yoffset
        };
       
        
        int num_newlines = tline_render(st, line, line_pos);
        num_newlines += 1; // Because the command which was echoed back is ignored.
        
        yoffset += num_newlines * (st->font.char_height + st->line_padding_y);
    }
}


