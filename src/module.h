#ifndef NEMI_MODULE_H
#define NEMI_MODULE_H

#include <stdint.h>
#include <stdbool.h>

// Modules are dynamically loaded libraries
// which can be used to modify the terminal emulator's 
// behaviour or add new features without needing to recompile libnemi.
// Modules can be easily removed or added.

// The module's events are automatically known at load time.
// by checking if the function exist or not.



#define MODULE_KEYBINDS_MAX 64

typedef struct NModuleKeybind_t {
    size_t key_hash;
    void(*fn_ptr)();
}
NModuleKeybind;


typedef struct NModule_t {
    char* path;
    void* address;

    // This functions is always needed by module.
    void(*fn_loaded)(size_t module_idx);
    void(*fn_quit)();

    NModuleKeybind keybinds [MODULE_KEYBINDS_MAX];
    size_t         num_keybinds;


    struct {
    
        void(*fn_render)();
        void(*fn_char_input)(char c);
        void(*fn_key_input)(int key, int modifiers);
        // ...

    }
    events;

}
NModule;

bool nmt_module_load(NModule* module, const char* path);
void nmt_module_quit(NModule* module);
bool nmt_is_module_loaded(NModule* module);


#endif 
