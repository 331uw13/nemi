#ifndef NEMI_H
#define NEMI_H

#include <stdint.h>


#include "leaf/leaf.h"
#include "nemi_config.h"
#include "terminal.h"
#include "script.h"
#include "render_buffer.h"
#include "log.h"

#define FLG_FONT_LOADED (1 << 0)

#define FLG_IGNORE_KEY_INPUT (1 << 1)
#define FLG_IGNORE_CHR_INPUT (1 << 2)



#define NEMI_TERMINALS_MAX 16
#define NEMI_KEYINBUF_MAX 8
#define NEMI_CHARINBUF_MAX 8
#define NEMI_SCRIPTS_MAX 32
#define NEMI_RENDERBUFS_MAX 32


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

    struct perl_script scripts [NEMI_SCRIPTS_MAX];

    struct render_buffer renderbufs [NEMI_RENDERBUFS_MAX];
    uint32_t             num_renderbufs;


    double frame_time;
    double frame_time_begin;
};


struct nemi* start_session(const char* config_file);
void         quit_session(struct nemi* st);

void zero_input_buffers(struct nemi* st);
void push_key_input(struct nemi* st, int key);
void push_char_input(struct nemi* st, char ch);

void begin_frame(struct nemi* st);
void end_frame(struct nemi* st);

bool key_down(struct nemi* st, int key);

// Convert column/row to window x/y position.
int coltox(struct nemi* st, int col);
int rowtoy(struct nemi* st, int row);

int new_renderbuf(struct nemi* st, int num_nodes_max);

void init_default_config(struct nemi* st);

#endif
