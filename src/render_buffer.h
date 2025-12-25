#ifndef NEMI_RENDER_BUFFER_H
#define NEMI_RENDER_BUFFER_H

#include <stddef.h>
#include <stdint.h>

#include "string.h"
#include "leaf/color_type.h"




enum rb_node_type : int {
    RBNODE_UNUSED = 0,
    RBNODE_MESH,
    RBNODE_TEXT
};

enum rb_coord_mode {
    RBCOORDMODE_CELL,      // Convert position to cell coordinates.
    RBCOORDMODE_ARBITRARY  // Nnormal X and Y. (0,0) to (win_width, win_height).
};

struct rb_node {
    enum rb_node_type type;
    union {
        struct {
            char     data [256];
            uint32_t len;
            int      pos_x, pos_y;
            struct   color_t color;
        }
        text;

        struct {
            float* vertices;
            size_t vertices_memsize;
        }
        mesh;
    };

    struct rb_node* prev;
    struct rb_node* next;
};

struct render_buffer {
    struct rb_node*     nodes;
    uint32_t            num_nodes;
    uint32_t            num_nodes_max;

    enum rb_coord_mode  coordinate_mode;
};




// 'int renderbuf_add...' functions return index to node.

struct nemi;

void renderbuf_remove_node(struct nemi* st,
        struct render_buffer* rb, int node_index);


int renderbuf_add_rect(struct nemi* st, 
        struct render_buffer* rb, int x, int y, int w, int h, int color);

int renderbuf_add_text(struct nemi* st, 
        struct render_buffer* rb, int x, int y, char* text, size_t len, int color);



void renderbuf_update_rect(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, int w, int h, int color);


void renderbuf_update_text(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, char* text, size_t len, int color);


        
//void renderbuf_test(struct nemi* st);

#endif
