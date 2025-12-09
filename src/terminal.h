#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include <stddef.h>

#include "tline.h"
#include "vec2.h"



#define TERMFLG_DROP_NEXT_READ (1 << 0)


struct term_cursor {
    struct vec2i pos;
};

struct terminal {
    int    flags;
    int    master_fd;
    pid_t  pid; // Child process PID.

    size_t buffer_limit; // How many lines can it hold.
    struct tline* lines;
    size_t        num_lines;
    size_t        num_lines_alloc;

    int           next_line_y;

    struct term_cursor cursor;
};



struct nemi;


struct terminal* spawn_terminal(struct nemi* st);
void             close_terminal(struct terminal* term);
void read_terminal(struct terminal* term, size_t* read_bytes, char* out, size_t mem_size);
void push_terminal_line(struct nemi* st, struct terminal* term, char* line_str, size_t line_len);
void render_terminal(struct nemi* st, struct terminal* term);
void execute_cmd(struct terminal* term, char* cmd_str, size_t cmd_len);
void move_cursor_to_end(struct terminal* term); // Move cursor to end of last line.


#endif
