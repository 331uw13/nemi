#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>
#include <assert.h>

#include "../../leaf.h"



static struct {
    
    struct fb_var_screeninfo vinfo;
   

    // This is kind of same as OpenGL's GL_SCISSOR_TEST
    struct {
        int x;
        int y;
        int width;
        int height;
        bool enabled;
    }
    scissor_test;
    

    LeafFramebuffer    main_fb;
    LeafFramebuffer*   active_fb;
    
    RGBColor fb_clear_color;
    int      fb_bytes_per_pixel;
}
g_lf = {
    .fb_clear_color = (RGBColor) { 0, 0, 0 },
    .fb_bytes_per_pixel = 0,
    .active_fb = NULL,
    .scissor_test.enabled = false
};


void p_leaf_set_activefb_pixel(size_t pixel_index, RGBColor color) {
    assert((pixel_index + 3) < g_lf.active_fb->memsize);
    // TODO: Scissor test is currently ignored.

    // TODO: This currently ignores 'bits_per_pixel' and color byte order.
    // it is different on some machines.
    g_lf.active_fb->address[ pixel_index + 0 ] = (color.r & 0xFF0000) >> 16;
    g_lf.active_fb->address[ pixel_index + 1 ] = (color.g & 0x00FF00) >> 8;
    g_lf.active_fb->address[ pixel_index + 2 ] = (color.b & 0x0000FF);
    g_lf.active_fb->address[ pixel_index + 3 ] = 255;

}

void p_leaf_set_activefb_pixel_xy(int x, int y, RGBColor color) {
    p_leaf_set_activefb_pixel(y + x * g_lf.active_fb->width, color);
}


LeafCtx* leaf_open (const char* fb_device, int width, int height, int flags) {
    (void)width;   // Its probably not ideal for this program to be able to
    (void)height;  // change the resolution, so just ignore with and height.
    (void)flags;

    LeafCtx* ctx = malloc(sizeof *ctx);
    if(!ctx) {
        goto out;
    }

    
    int fbdev_fd = open(fb_device, O_RDWR);
    if(fbdev_fd < 0) {
        fprintf(stderr, "%s: %s(): Failed to open framebuffer device: '%s' | %s\n",
                __FILE__, __func__,
                fb_device,
                strerror(errno));

        if(errno == EACCES) {
            // TODO: Print group?
            fprintf(stderr, "Check if your user is in same group as '%s'\n",
                    fb_device);
        }

        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }


    ioctl(fbdev_fd, FBIOGET_VSCREENINFO, &g_lf.vinfo);
    
    ctx->win_width  = g_lf.vinfo.xres;
    ctx->win_height = g_lf.vinfo.yres;

    g_lf.main_fb = (LeafFramebuffer) {
        .address = NULL,
        .memsize = 0,
        .width   = g_lf.vinfo.xres,
        .height  = g_lf.vinfo.yres
    };



    g_lf.fb_bytes_per_pixel = g_lf.vinfo.bits_per_pixel / 8;

    // Map framebuffer device into memory.
    g_lf.main_fb.memsize = g_lf.vinfo.yres * g_lf.vinfo.xres * g_lf.fb_bytes_per_pixel;
    g_lf.main_fb.address 
        = mmap(0, g_lf.main_fb.memsize,
                PROT_READ | PROT_WRITE, MAP_SHARED, fbdev_fd, (off_t)0);

    if(g_lf.main_fb.address == MAP_FAILED) {
        fprintf(stderr, "%s: %s:() Failed to map framebuffer into memory | %s\n",
                __FILE__, __func__, strerror(errno));
        leaf_quit(ctx);
        ctx = NULL;
        goto out;
    }

    g_lf.active_fb = &g_lf.main_fb;

    close(fbdev_fd);
    
out:
    return ctx;
}

void leaf_quit(LeafCtx* ctx) {

    if(g_lf.main_fb.address != NULL
    && g_lf.main_fb.address != MAP_FAILED) {
        munmap(g_lf.main_fb.address, g_lf.main_fb.memsize);
    }

    free(ctx);
}

bool leaf_should_quit() {
    // TODO: Is there actually reasonable event which should close the program?
    // We are not in desktop environment so its not a window which can close...
    return false;
}


void leaf_swap_buffers() {
    // Nothing to do.
}


void leaf_get_events() {
    // TODO: Get user input.
}

void leaf_set_viewport(int x, int y, int w, int h) {
    // Nothing to do.
}


void leaf_hide_mouse(bool is_hidden) {
    (void)is_hidden;
    // Nothing to do.
}

    
// TODO: Implement this.
void leaf_enable_vsync(bool is_enabled) {
    (void)is_enabled;
}


// TODO: Implement this.
bool leaf_key_down(int key) {
    return false;
}

double leaf_get_time_insec() {
    return (double)clock() / CLOCKS_PER_SEC;
}

void leaf_clear_color(RGBAColor color) {
    g_lf.fb_clear_color = (RGBColor) {
        color.r,
        color.g,
        color.b
    };
}

void leaf_clear() {
    if(g_lf.scissor_test.enabled) {
        const int y_beg = g_lf.scissor_test.y;
        const int x_beg = g_lf.scissor_test.x;
        const int y_end = g_lf.scissor_test.y + g_lf.scissor_test.height;
        const int x_end = g_lf.scissor_test.x + g_lf.scissor_test.width;
        for(int y = y_beg; y < y_end; y++) {
            for(int x = x_beg; x < x_end; x++) {
                p_leaf_set_activefb_pixel_xy(x, y, g_lf.fb_clear_color);
            }
        }
    }
    else {
        for(size_t i = 0; i < g_lf.active_fb->memsize; i++) {
            p_leaf_set_activefb_pixel(i, g_lf.fb_clear_color);
        }
    }
}

void leaf_enable_scissor_test (bool is_enabled) {
    g_lf.scissor_test.enabled = is_enabled;
}

void leaf_set_scissor_region  (int x, int y, int w, int h) {
    g_lf.scissor_test.x = x;
    g_lf.scissor_test.y = y;
    g_lf.scissor_test.width = w;
    g_lf.scissor_test.height = h;
}


bool leaf_create_framebuffer(LeafFramebuffer* fb, uint32_t width, uint32_t height) {
    fb->memsize = width * height * g_lf.fb_bytes_per_pixel;
    fb->address = calloc(sizeof *fb->address, fb->memsize);
    assert(fb->address != NULL);

    fb->width = width;
    fb->height = height;

    return true;
}

void leaf_use_framebuffer(LeafFramebuffer* fb) {
    if(fb == NULL) {
        g_lf.active_fb = &g_lf.main_fb;
    }
    else {
        g_lf.active_fb = fb;
    }
}

void leaf_free_framebuffer(LeafFramebuffer* fb) {
    if(fb->address != NULL) {
        free(fb->address);
        fb->address = NULL;
    }
}

LeafTexture leaf_load_texture(const char* path) {
    assert("leaf_load_texture is not yet implemented (GRAPHICS=linux_fbdev)");

    // Unreachable.
    return (LeafTexture){};
}

