#ifndef NEMI_H
#define NEMI_H

#include <stdint.h>

#include "leaf/leaf.h"
#include "terminal.h"
#include "tline.h"

#define FLG_FONT_LOADED (1 << 0)

#define NEMI_TERMINALS_MAX 16
#define NEMI_KEYINBUF_MAX 8
#define NEMI_CHARINBUF_MAX 8


struct nemi_config {
    int line_padding_y;
    int rows_end_padding;
    float scroll_y_mult;
    float scroll_x_mult;
};

struct nemi {
    int flags;
    struct leaf_ctx_t* lfctx;
    struct font_t      font;

    struct nemi_config cfg;

    struct terminal    terminals [NEMI_TERMINALS_MAX];
    struct terminal*   terminal; // Current terminal.
    uint16_t           num_terminals;    

    struct rgb_color   palette [NUM_CHAR_COLORS];

    // User input buffers.
    int  last_key_in;
    char last_char_in;
    int  key_inputs  [NEMI_KEYINBUF_MAX];
    char char_inputs [NEMI_CHARINBUF_MAX];
};


struct nemi* start_session();
void         quit_session(struct nemi* st);

void         zero_input_buffers(struct nemi* st);
void         push_key_input(struct nemi* st, int key);
void         push_char_input(struct nemi* st, char ch);
void         end_frame(struct nemi* st);
void         to_grid_pos(struct nemi* st, int* x, int* y);
bool         key_down(struct nemi* st, int key);
void             set_palette_color(struct nemi* st, int color_id, struct rgb_color color);
struct rgb_color get_palette_color(struct nemi* st, int color_id);


// Returns pointer to 'buffer' where to continue reading.
char* handle_osc_escseq(struct nemi* st, char* ptr, char* buffer, size_t size);

void         init_default_palette(struct nemi* st);
void         init_default_config(struct nemi* st);

#endif
