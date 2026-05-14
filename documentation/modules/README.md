Modules are dynamically loaded libraries. They are linked with the main library `libnemi.so`

## Loading
At the moment the directory which nemi loads all modules is `./modules`.

It treats all files as modules which start with ELF format magic bytes.


## Compiling a module
Simple example build script: https://github.com/331uw13/nemi/blob/main/modules/buildmod.sh


# Events

Event can be enabled by creating a function implementation with a specific name.

Available events:
```c
void module_event_render();
void module_event_key_input(int key, int key_modifiers);
void module_event_char_input(char ch);
```

# Examples
## "Hello world"
```c
#include "nemi.h"
#include "log.h"

// All modules need this function. It is called when the module has been loaded succesfully.
// it also gives you the module index.
void module_load(size_t module_idx) {
  logprintf(LOG_INFO, "Hello world!");
}

// This function is optional. can be used to free allocated memory.
void module_quit() {
  logprintf(LOG_INFO, "Bye bye world...");  
}
```

## Keybinds
```c
#include "nemi.h"
#include "log.h"

// Nemi uses GLFW under the hood,
// you can find the key definitions from https://www.glfw.org/docs/latest/group__keys.html
static const int keybind_test
= [] {
  GLFW_KEY_A,
  GLFW_KEY_LEFT_SHIFT
};

void keybind_test_func() {
  logprintf(LOG_INFO, "Keybind was pressed!");
}

void module_load(size_t module_idx) {
  Nemi* st = nmt_getst(); // Global state pointer.
  nmt_assign_module_keybind(st, module_idx, keybind_test_func, keybind_test, ARRAY_LEN(keybind_test));
}

// This function is optional. can be used to free allocated memory.
/*
void module_quit() {
}
*/
```

## Shape Rendering
```c
#include "nemi.h"
#include "log.h"


void module_event_render() {
  // "leaf" is very simple and pretty lightweight OpenGL and freetype2 utility.
  // It was mainly written for this project :)

  // Nemi has a different framebuffer for this kind of rendering.
  // It is displayed behind the terminal cells.

  // You can find more draw calls from: https://github.com/331uw13/nemi/blob/main/src/leaf/draw.h
  // ----------------------------------

  float x = 100.0f;
  float y = 100.0f;
  float radius = 50.0f;
  int num_triangles = 8;
  struct color_t color = (struct color_t) { 255, 255, 255 };
  leaf_draw_circle(x, y, radius, num_triangles, color);
}

void module_load(size_t module_idx) {
}


/*
void module_quit() {
}
*/
```

## Image Rendering
```c
#include "nemi.h"

static uint32_t texture = 0;
static int texture_width = 0;
static int texture_height = 0;

    
// The image is rendered
// behind terminal cells.

void module_event_render() {
    leaf_draw_texture_rect
    (
        0,
        0,
        texture_width,
        texture_height,
        texture,
        (struct color_t) { 200, 200, 200 }, // Texture tint.
        LEAF_TEXTURE_NO_OPTIONS // Read 'src/leaf/draw.h' for available options.
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
```
