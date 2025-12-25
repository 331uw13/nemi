#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include "vterm.h"

#include "vec2.h"
#include "string.h"


#define VMODE_WORD_MAX 64  // TODO Add to config.
#define VMODE_WORD_SEPARATORS_MAX 16  // TODO: Add to config.

struct scrollback_row {
    VTermScreenCell* cells;
    uint32_t         num_cells;
    uint32_t         num_cells_alloc;
};

struct scrollback {
    struct scrollback_row* rows;
    size_t num_rows;
    size_t num_rows_max;
    int    offset;
};

enum term_write_target {
    TERM_WRITE_PTY,
    TERM_WRITE_VTERM
};

/*
#define VMODE_MODE_FILES     'F'   // Interact with files.
#define VMODE_MODE_SEL       'S'   // Select mode.
#define VMODE_MODE_BLOCK_SEL 'B'   // Block select mode.
*/

struct terminal {
    int          flags;
    int          master_fd;
    pid_t        pid;
    VTerm*       vt;
    VTermScreen* vtscrn;
    VTermState*  vtstate;
    bool         is_altscreen;

    struct scrollback sb;
    
    int rows;
    int cols;
    int line_height;

    float blink_timer;
    /*
    // Visual mode.
    struct {
        int  mode;
        bool enabled;
        bool was_enabled_before_altscreen;

        VTermPos curs;

        int  sel_start_row;
        int  sel_start_col;

        // Which characters define separators for word.
        char     word_separators [VMODE_WORD_SEPARATORS_MAX];
        uint32_t num_word_separators;

        // Word where vmode cursor is on.
        // Not valid if select region is enabled.
        char     word [VMODE_WORD_MAX];
        uint32_t word_len;
    }
    vmode;
    */
};


struct nemi;

struct terminal* spawn_terminal(struct nemi* st, int rows, int cols);
void             close_terminal(struct terminal* term);

void read_terminal         (struct nemi* st, struct terminal* term);
void render_terminal       (struct nemi* st, struct terminal* term);
void write_term            (struct terminal* term, enum term_write_target target, char* fmt, ...);
void terminal_scroll       (struct terminal* term, int offset);
void terminal_set_scroll   (struct terminal* term, int scroll);
void update_terminal_blink_timer  (struct nemi* st, struct terminal* term);

void terminal_handle_char_event   (struct nemi* st, struct terminal* term);
void terminal_handle_key_event    (struct nemi* st, struct terminal* term);
void terminal_handle_resize_event (struct nemi* st, struct terminal* term);
void terminal_handle_altscreen_change_event(struct nemi* st, struct terminal* term);


void terminal_init_palette        (struct nemi* st, struct terminal* term);

#endif
