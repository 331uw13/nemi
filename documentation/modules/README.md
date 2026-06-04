Modules are dynamically loaded libraries. They are linked with the main library `libnemi.so` 

## Loading
At the moment the directory which nemi loads all modules is `./modules`.

It treats all files as modules which start with ELF format magic bytes.


## Compiling a module
The plan is to support many graphics APIs, its recommended to read and understand the example build script
for modules

* You can find the full `buildmodule.sh` script from `nemi/modules/` directory.
* This task will be automated in the near future, so no manual compiling is needed.

```bash

module_name=$1
module_cfiles=$2

# The directory which libnemi.so lives, must be absolute.
# This assumes that the script is compiled in "nemi/modules/" directory.
# but you probably get the point...
libnemi_dir="$(cd .. && pwd)"

# Source file directory of nemi.
libnemi_srcdir="../src"

# This can be also set to "GRAPHICS_LINUX_FBDEV" in the future,
# to use the linux framebuffer device.
# But the implementation is not ready yet.
graphics_backend="GRAPHICS_OPENGL" 

gcc $module_cfiles \
    -D$graphics_backend \
    -shared \
    -fPIC \
    -Wl,-rpath=$libnemi_dir \
    -L$libnemi_dir \
    -lnemi \
    -I$libnemi_srcdir \
    -o $module_name 
```

# Events

Event can be enabled by creating a function implementation with a specific name. 

Available events:
```c
void module_event_render();
void module_event_key_input(int key, int key_modifiers);
void module_event_char_input(char ch);
void module_event_window_resized();
void module_event_mouse_moved(float new_x, float new_y);
void module_event_mouse_pressed(int button);
void module_event_mouse_scroll(int direction);
```

# Examples
## "Hello world"
```c
#include "nemi.h"

// All modules need this function. It is called when the module has been loaded succesfully.
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
#include "leaf/keyboard.h"

// Nemi uses GLFW under the hood,
// you can find the key definitions from "src/leaf/keyboard.h"
static const int keybind_test
= [] {
  KEYBOARD_KEY_A,
  KEYBOARD_KEY_LEFT_SHIFT
};

void keybind_test_func() {
  logprintf(LOG_INFO, "Keybind was pressed!");
}

void module_load(size_t module_idx) {
  Nemi* st = nmt_getst(); // Global state pointer.
  nmt_assign_module_keybind(st, module_idx, keybind_test_func, keybind_test, ARRAY_LEN(keybind_test));
}
```

## Shape Rendering
```c
#include "nemi.h"


void module_event_render() {
  // "leaf" is a graphics utility. and graphics api wrapper to be able to use multiple backends.
  // It was mainly written for this project :)

  // Nemi has a different framebuffer for this kind of rendering.
  // It is displayed behind the terminal cells.

  // You can find more draw calls from: "nemi/src/leaf/draw.h"
  // ---------------------------------------------------------

  float x = 100.0f;
  float y = 100.0f;
  float radius = 50.0f;
  int num_triangles = 8;
  leaf_draw_circle(x, y, radius, num_triangles, (RGBColor){ 255, 255, 255 });
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
        (RGBColor) { 200, 200, 200 }, // Texture tint.
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
