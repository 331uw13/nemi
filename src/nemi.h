#ifndef NEMI_H
#define NEMI_H

#include <stdint.h>

#include "leaf/leaf.h"
#include "terminal.h"

#define FLG_FONT_LOADED (1 << 0)

#define NEMI_TERMINALS_MAX 16
#define NEMI_KEYINBUF_MAX 8
#define NEMI_CHARINBUF_MAX 8


enum nemi_config_colors {
   
    NEMI_COLOR_BLACK,
    NEMI_COLOR_RED,
    NEMI_COLOR_GREEN,
    NEMI_COLOR_YELLOW,
    NEMI_COLOR_BLUE,
    NEMI_COLOR_MAGENTA,
    NEMI_COLOR_CYAN,
    NEMI_COLOR_WHITE,

    NEMI_BRIGHT_COLOR_BLACK,
    NEMI_BRIGHT_COLOR_RED,
    NEMI_BRIGHT_COLOR_GREEN,
    NEMI_BRIGHT_COLOR_YELLOW,
    NEMI_BRIGHT_COLOR_BLUE,
    NEMI_BRIGHT_COLOR_MAGENTA,
    NEMI_BRIGHT_COLOR_CYAN,
    NEMI_BRIGHT_COLOR_WHITE,
    
    NEMI_COLOR_BG,
    NEMI_COLOR_FG,

    NEMI_COLOR_COUNT
};

struct nemi_config {
    int padding_x;
    int padding_y;
    int line_padding;

    struct color_t colors [NEMI_COLOR_COUNT];
};

struct nemi {
    int flags;
    struct leaf_ctx_t* lfctx;
    struct font_t      font;

    struct nemi_config cfg;

    struct terminal    terminals [NEMI_TERMINALS_MAX];
    struct terminal*   terminal; // Current terminal.
    uint16_t           num_terminals;    

    int win_rows;
    int win_cols;

    // User input buffers.
    int  last_key_in;
    char last_char_in;
    int  key_inputs  [NEMI_KEYINBUF_MAX];
    char char_inputs [NEMI_CHARINBUF_MAX];
};


struct nemi* start_session();
void         quit_session(struct nemi* st);

void zero_input_buffers(struct nemi* st);
void push_key_input(struct nemi* st, int key);
void push_char_input(struct nemi* st, char ch);

void begin_frame(struct nemi* st);
void end_frame(struct nemi* st);

void to_grid_pos(struct nemi* st, int* x, int* y);
bool key_down(struct nemi* st, int key);

// Convert column/row to window x/y position.
int coltox(struct nemi* st, int col);
int rowtoy(struct nemi* st, int row);


//void         init_default_palette(struct nemi* st);
void         init_default_config(struct nemi* st);

#endif
