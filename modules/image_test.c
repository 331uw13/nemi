#include "nemi.h"



static uint32_t texture = 0;
static int texture_width = 0;
static int texture_height = 0;



void module_event_render() {
    leaf_draw_texture_rect
    (
        0, 
        0,
        texture_width,
        texture_height,
        texture,
        (struct color_t) { 200, 200, 200 },
        LEAF_TEXTURE_NO_OPTIONS
    );
}

void module_loaded(size_t module_idx) {

    texture = leaf_load_texture
    (
        "img.jpg",
        &texture_width,
        &texture_height
    );

}


void module_quit() {
    glDeleteTextures(1, &texture);
}


