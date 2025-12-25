
#include <stdio.h>
#include "render_buffer.h"
#include "nemi.h"


    /*
static
int renderbuf_find_free_mesh(struct render_buffer* rb) {
    return -1;
    int index = rb->next_mesh_index;

    struct vertex_mesh* next_free_mesh = &rb->meshes[index];
    if(next_free_mesh->vertices) {
        // The next possible mesh is not free. Find one.

        for(uint32_t i = rb->num_meshes_max; i > 0; i--) {
            if(!rb->meshes[i].vertices) {
                index = i;
                goto found;
            }
        }

        index = -1;
    }
    else {
        // Mesh at 'next_mesh_index' was free,
        // need to move it for next call.
        rb->next_mesh_index++;
        rb->next_mesh_index %= rb->num_meshes_max;
    }

found:
    return index;
}

    */
/*



void renderbuf_add_rect_to_mesh(struct nemi* st, 
        struct vertex_mesh* mesh,
        int x, int y, int w, int h, int color) {

    uint32_t stride_size = 2// x,y  + 3; // r,g,b
    stride_size *= sizeof *mesh->vertices;

    mesh->vertices_memsize = stride_size * 6;
    mesh->vertices = malloc(mesh->vertices_memsize);

    float xf = m_normalize_X(st, (float)x);
    float yf = m_normalize_Y(st, (float)y);
    float wf = (float)w / (float)st->lfctx->win_width;
    float hf = (float)h / (float)st->lfctx->win_height;

    float red = ((color & (0xFF0000)) >> 16) / 255.0f;
    float grn = ((color & (0x00FF00)) >> 8)  / 255.0f;
    float blu =  (color & (0x0000FF))        / 255.0f;
    
    float vertices[] = {
        xf,    yf-hf, red,grn,blu,
        xf,    yf,    red,grn,blu,
        xf+wf, yf,    red,grn,blu,

        xf,    yf-hf, red,grn,blu,
        xf+wf, yf,    red,grn,blu,
        xf+wf, yf-hf, red,grn,blu
    };

    memcpy(mesh->vertices, vertices, sizeof(vertices));
}
*/

// Normalize (0,0, win_w/h)  ->  (-1.0, +1.0)
static
float m_normalize_X(struct nemi* st, float x) {
    return (x / (float)st->lfctx->win_width) * 2.0 - 1.0f; 
}

static
float m_normalize_Y(struct nemi* st, float y) {
    y = st->lfctx->win_height - y;
    return (y / (float)st->lfctx->win_height) * 2.0 - 1.0f; 
}

#define HEX2RED_CHANNEL(num) ((num & 0xFF0000) >> 16)
#define HEX2GRN_CHANNEL(num) ((num & 0x00FF00) >> 8)
#define HEX2BLU_CHANNEL(num)  (num & 0x0000FF)

static
void renderbuf_rect_node(struct nemi* st, 
        struct render_buffer* rb,
        struct rb_node* node,
        int x, int y, int w, int h, int color) {
    node->type = RBNODE_MESH;

    if(rb->coordinate_mode == RBCOORDMODE_CELL) {
        x = coltox(st, x);
        y = rowtoy(st, y);
        w *= st->font.char_width * 2;
        h *= st->font.char_height;
    }

    uint32_t stride_size = 2 /* x, y */ + 3; /* r, g, b */
    stride_size *= sizeof *node->mesh.vertices;

    // Reallocate if memory size dont match.
    // Required vertices memory size.
    uint32_t req_memsize = stride_size * 6;

    if(node->mesh.vertices && node->mesh.vertices_memsize != req_memsize) {
        node->mesh.vertices = realloc(node->mesh.vertices, req_memsize);
    }
    else {
        node->mesh.vertices = malloc(req_memsize);    
    }

        
    node->mesh.vertices_memsize = req_memsize;
    float xf = m_normalize_X(st, (float)x);
    float yf = m_normalize_Y(st, (float)y);
    float wf = (float)w / (float)st->lfctx->win_width;
    float hf = (float)h / (float)st->lfctx->win_height;

    float red = (float)HEX2RED_CHANNEL(color) / 255.0f;
    float grn = (float)HEX2GRN_CHANNEL(color) / 255.0f;
    float blu = (float)HEX2BLU_CHANNEL(color) / 255.0f;
    
    float vertices[] = {
        xf,    yf-hf, red,grn,blu,
        xf,    yf,    red,grn,blu,
        xf+wf, yf,    red,grn,blu,

        xf,    yf-hf, red,grn,blu,
        xf+wf, yf,    red,grn,blu,
        xf+wf, yf-hf, red,grn,blu
    };
   
    memcpy(node->mesh.vertices, vertices, sizeof vertices);
}

static
void renderbuf_text_node(struct nemi* st,
        struct render_buffer* rb,
        struct rb_node* node,
        int x, int y, char* text, size_t len, int color) {
    node->type = RBNODE_TEXT;

    if(rb->coordinate_mode == RBCOORDMODE_CELL) {
        x = coltox(st, x);
        y = rowtoy(st, y);
    }

    if(len >= sizeof(node->text.data)-2) {
        len = sizeof(node->text.data)-2;
    }

    memcpy(node->text.data, text, len);
    node->text.data[ sizeof(node->text.data)-1 ] = '\0';

    node->text.color.r = HEX2RED_CHANNEL(color);
    node->text.color.g = HEX2GRN_CHANNEL(color);
    node->text.color.b = HEX2BLU_CHANNEL(color);

    node->text.pos_x = x;
    node->text.pos_y = y;
    node->text.len = len;
}

/*


        0   1   2   3   4   5   6
 NULL - O - O - O - O - O - O - O - NULL

                ^
                |
 Remove Index 2 '
     ( rb->nodes + 2 )
                     |
                     v

        0   1        2         3   4   5   6
 NULL - O - O  (NULL-X-NULL)   O - O - O - O - NULL
            `------------------'

*/



void renderbuf_remove_node(struct nemi* st, struct render_buffer* rb, int node_index) {
    struct rb_node* node = rb->nodes + node_index;

    if(!node->prev && !node->next) {
        logprintf(LOG_WARN, "Trying to remove already removed node (%i)", node_index);
        return;
    }

    if(node->prev) {
        node->prev->next = node->next;
    }

    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
    node->type = RBNODE_UNUSED;
}

static
bool is_node_active(struct rb_node* node) {
    return !(node->next && node->prev);
}

static
struct rb_node* renderbuf_add_node(struct render_buffer* rb, int* index_out) {
    int this_index = 0;

    if(rb->num_nodes+1 < rb->num_nodes_max) {
        this_index = rb->num_nodes;
    }
    else {
        // List may be full
        // but we can try to find empty nodes
        // in case of fragmentation happened.

        for(size_t i = 0; i < rb->num_nodes_max; i++) {
            if(!is_node_active(&rb->nodes[i])) {
                this_index = i;
                goto found_free_node;
            }
        }

        return NULL; // The list is really full.
    }


found_free_node:

    struct rb_node* node = rb->nodes + this_index;
        
    if(rb->num_nodes > 0) {
        node->prev = rb->nodes + (rb->num_nodes - 1);
        node->prev->next = node;
    }

    *index_out = this_index;

    if(rb->num_nodes+1 < rb->num_nodes_max) {
        rb->num_nodes++;
    }
    return node;
}


int renderbuf_add_rect(struct nemi* st, 
        struct render_buffer* rb, int x, int y, int w, int h, int color) {
    int ret_index = -1;

    struct rb_node* node = renderbuf_add_node(rb, &ret_index);
    if(!node) {
        logprintf(LOG_ERROR, "Render buffer is full.");
    }
   
    renderbuf_rect_node(st, rb, node, x, y, w, h, color);
    return ret_index;
}


int renderbuf_add_text(struct nemi* st, 
        struct render_buffer* rb, int x, int y, char* text, size_t len, int color) {
    int ret_index = -1;
    
    struct rb_node* node = renderbuf_add_node(rb, &ret_index);
    if(!node) {
        logprintf(LOG_ERROR, "Render buffer is full.");
    }

    renderbuf_text_node(st, rb, node, x, y, text, len, color);
    return ret_index;
}


void renderbuf_update_rect(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, int w, int h, int color) {
    if(node_index >= rb->num_nodes_max) {
        return;
    }

    struct rb_node* node = rb->nodes + node_index;
    if(is_node_active(node) && node->type == RBNODE_MESH) {
        renderbuf_rect_node(st, rb, node, x, y, w, h, color);
    }
}

void renderbuf_update_text(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, char* text, size_t len, int color) {
    if(node_index >= rb->num_nodes_max) {
        return;
    }

    struct rb_node* node = rb->nodes + node_index;
    if(is_node_active(node) && node->type == RBNODE_TEXT) {
        renderbuf_text_node(st, rb, node, x, y, text, len, color);
    }
}

