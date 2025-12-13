#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include <stddef.h>

#include "tline.h"
#include "vec2.h"
#include "string.h"


#define NO_INPUT_ECHO          (1 << 0)
#define NO_AUTO_CURSOR_MOVE_X  (1 << 1)
#define NO_AUTO_CURSOR_MOVE_Y  (1 << 2)

#define TERM_TITLE_MAX 32


struct term_cursor {
    struct vec2i pos;
    // May need to add something here in the future.
};

struct terminal {
    int    flags;
    int    master_fd;
    pid_t  pid; // Child process PID.

    char   title [TERM_TITLE_MAX];
    int    line_height;

    struct tline* lines;
    struct tline* currln;

    size_t        num_lines;
    size_t        num_lines_alloc;
    size_t        num_added_tchars;
    int           rows; // How many rows/lines can be shown at once.
    int           cols; // How many chars can be shown at once per line.

    struct vec2i       scroll;
    struct term_cursor curs;
    
    struct string_t    icmd;        // Internal command prompt.
};



struct nemi;

struct terminal* spawn_terminal(struct nemi* st);
void             close_terminal(struct terminal* term);
bool terminal_prep_lines_add(struct terminal* term, int num_add);

void read_terminal         (struct nemi* st, struct terminal* term);
void terminal_add_chars    (struct nemi* st, struct terminal* term, char* buffer, size_t size);
void render_terminal       (struct nemi* st, struct terminal* term);
void execute_cmd           (struct terminal* term, char* cmd_str, size_t cmd_len);
void set_terminal_title    (struct terminal* term, char* buffer, size_t len);
void move_curs_to          (struct terminal* term, int x, int y);
void move_curs_off         (struct terminal* term, int xoff, int yoff);
void scroll_terminal_down  (struct nemi* st, struct terminal* term);
void scroll_terminal       (struct nemi* st, struct terminal* term, struct vec2i offset);
bool terminal_onlastpage   (struct terminal* term);
void terminal_clear        (struct terminal* term);
void terminal_push_cursmov (struct terminal* term, struct vec2i cursor_pos);

struct tline*  get_terminal_lastln(struct terminal* term);

void terminal_handle_resize_event (struct nemi* st, struct terminal* term);
void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_data_event   (struct nemi* st, struct terminal* term);

// Returns pointer to 'buffer' where to continue reading
// or 'NULL' if it was not valid cursor or erase control sequence.
char* terminal_handle_csi
    (struct nemi* st, struct terminal* term, char* ptr, char* buffer, size_t size);


#endif
