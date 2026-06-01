/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/
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
        void(*fn_window_resized)();
        // ...

    }
    events;


    // At module load, this equals to process's current directory.
    char* current_dir;
}
NModule;

bool nmt_module_load(NModule* module, const char* path);
void nmt_module_quit(NModule* module);
bool nmt_is_module_loaded(NModule* module);

// This function changes NModule.current_dir, and not the process's current directory.
// 'relative_path' also can change to parent directories with ".."
// and they can be concatenated like with cd command: "../.."
// This was done because if many modules are loaded and they all mess with the process's
// current directory, can lead to alot of problems.
//
// NModule.current_dir is left unchanged if the result directory cannot be accessed.
bool nmt_module_chdir_rel(NModule* module, const char* relative_path);
bool nmt_module_chdir_abs(NModule* module, const char* absolute_path);



#endif 
