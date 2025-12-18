#ifndef NEMI_RENDER_BUFFER_H
#define NEMI_RENDER_BUFFER_H

#include <stddef.h>
#include <stdint.h>

#include "string.h"





enum rb_node_type : int {
    RBNODE_UNUSED = 0,
    RBNODE_MESH,
    RBNODE_TEXT
};

struct rb_node {
    enum rb_node_type type;
    union {
        struct {
            // ... TODO ...
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
};



struct nemi;

void renderbuf_remove_node(struct nemi* st,
        struct render_buffer* rb, int node_index);

int renderbuf_add_rect(struct nemi* st, 
        struct render_buffer* rb, int x, int y, int w, int h, int color);

void renderbuf_update_rect(struct nemi* st, 
        struct render_buffer* rb, int mesh_index, int x, int y, int w, int h, int color);

        
//void renderbuf_test(struct nemi* st);

#endif
