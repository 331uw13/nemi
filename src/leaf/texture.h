#ifndef LEAF_TEXTURE_H
#define LEAF_TEXTURE_H

#include <stdint.h>



#ifdef GRAPHICS_OPENGL
typedef struct LeafTexture_t {
    uint32_t id;
    int width;
    int height;
}
LeafTexture;
#endif // GRAPHICS_OPENGL



#ifdef GRAPHICS_LINUX_FBDEV
typedef struct LeafTexture_t {
    
}
LeafTexture;
#endif // GRAPHICS_LINUX_FBDEV


#endif
