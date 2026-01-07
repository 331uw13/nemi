
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
    node->layer = RBNODE_LAYER_FIRST;

    if(rb->coordinate_mode == RBCOORDMODE_CELL) {
        x = coltox(st, x);
        y = rowtoy(st, y);
        w *= st->font.char_width * 2;
        h *= st->font.char_height * 2;
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
    node->layer = RBNODE_LAYER_LAST;

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
static
void print_node(struct rb_node* node) {
    printf(" Node \033[34m%p\033[0m, Used = %s\n", node, node->type == RBNODE_UNUSED ? "No" : "Yes");
    printf(" `- Prev = \033[2;35m%p\033[0m\n", node->prev);
    printf(" `- Next = \033[2;35m%p\033[0m\n", node->next);
}

static
void print_renderbuf_nodes(struct render_buffer* rb) {
    printf(" %s \n-----------------------------------------------\n", __func__);
    for(uint32_t i = 0; i < rb->num_nodes_max; i++) {
        print_node(&rb->nodes[i]);
    }

    printf("==============\n");
    printf("Head = %p\n", rb->node_link_head);
    printf("Tail = %p\n", rb->node_link_tail);
    printf("==============\n");
    printf("----------------------------------------\n");
}
*/

void renderbuf_remove_node(struct nemi* st, struct render_buffer* rb, int node_index) {
    if(rb->num_nodes == 0) {
        logprintf(LOG_ERROR, "Trying to remove render buffer node (node_index = %i) from empty render buffer.",
                node_index);
        return;
    }
    if(node_index < 0 || (uint32_t)node_index >= rb->num_nodes_max) {
        logprintf(LOG_ERROR, "Invalid renderbuffer 'node_index' %i cant be removed.", node_index);
        return;
    }

    struct rb_node* node = &rb->nodes[node_index];
    if(node->type == RBNODE_UNUSED) {
        logprintf(LOG_WARN, "Trying to remove already unused renderbuffer node (node_index = %i)", node_index);
        return;
    }

    if(node->prev) {
        if(!(node->prev->next = node->next)) {
            rb->node_link_tail = node->prev;
        }
    }
    if(node->next) {
        if(!(node->next->prev = node->prev)) {
            rb->node_link_head = node->next;
        }
    }

    node->type = RBNODE_UNUSED;
    node->next = NULL;
    node->prev = NULL;

    rb->num_nodes--;
    //print_renderbuf_nodes(rb);
}

static
bool is_node_active(struct rb_node* node) {
    return !(node->next && node->prev);
}


static
struct rb_node* renderbuf_add_node(struct render_buffer* rb, int* index_out) {
    int this_index = -1;

    for(size_t i = 0; i < rb->num_nodes_max; i++) {
        if(rb->nodes[i].type == RBNODE_UNUSED) {
            this_index = i;
            break;
        }
    }

    if(this_index < 0) {
        return NULL; // No free node was found.
    }
    
    struct rb_node* node = &rb->nodes[this_index];
    

    for(int64_t i = this_index+1; i < rb->num_nodes_max; i++) {
        struct rb_node* next_node = &rb->nodes[i];
        if(next_node->type != RBNODE_UNUSED) {
            node->next = next_node;
            node->next->prev = node;
            break;
        }
    }

    for(int64_t i = this_index-1; i >= 0; i--) {
        struct rb_node* prev_node = &rb->nodes[i];
        if(prev_node->type != RBNODE_UNUSED) {
            node->prev = prev_node;
            node->prev->next = node;
            break;
        }
    }


    if(node->prev == NULL) {
        rb->node_link_head = node;
    }
    if(node->next == NULL) {
        rb->node_link_tail = node;
    }

    rb->num_nodes++;
    *index_out = this_index;
    //node->type = RBNODE_UNUSED;
    return node;
}


int renderbuf_add_rect(struct nemi* st, 
        struct render_buffer* rb, int x, int y, int w, int h, int color) {
    int ret_index = -1;

    struct rb_node* node = renderbuf_add_node(rb, &ret_index);
    if(!node) {
        logprintf(LOG_ERROR, "Render buffer is full.");
        return -1;
    }

    
    renderbuf_rect_node(st, rb, node, x, y, w, h, color);
    
    logprintf(LOG_INFO, "Created rect rbnode %i", ret_index);
    //print_renderbuf_nodes(rb);
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
    
    logprintf(LOG_INFO, "Created text rbnode %i", ret_index);
    return ret_index;
}


void renderbuf_update_rect(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, int w, int h, int color) {
    if(node_index >= rb->num_nodes_max) {
        return;
    }

    struct rb_node* node = rb->nodes + node_index;
    if(node->type == RBNODE_MESH) {
        renderbuf_rect_node(st, rb, node, x, y, w, h, color);
    }
}

void renderbuf_update_text(struct nemi* st, 
        struct render_buffer* rb, uint32_t node_index, int x, int y, char* text, size_t len, int color) {
    if(node_index >= rb->num_nodes_max) {
        return;
    }

    struct rb_node* node = rb->nodes + node_index;
    if(node->type == RBNODE_TEXT) {
        renderbuf_text_node(st, rb, node, x, y, text, len, color);
    }
}

