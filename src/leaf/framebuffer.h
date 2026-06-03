#ifndef LEAF_FRAMEBUFFER_H
#define LEAF_FRAMEBUFFER_H

#include <stdint.h>


#ifdef GRAPHICS_OPENGL
typedef struct LeafFramebuffer_t {
    uint32_t texture;
    uint32_t fbo;
    uint32_t rbo;
    uint32_t width;
    uint32_t height;
}
LeafFramebuffer;
#endif // GRAPHICS_OPENGL


#ifdef GRAPHICS_LINUX_FBDEV
typedef struct LeafFramebuffer_t {
    uint8_t* address;
    size_t   memsize;
    uint32_t width;
    uint32_t height;
}
LeafFramebuffer;
#endif // GRAPHICS_LINUX_FBDEV




#endif
