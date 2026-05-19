#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "module.h"
#include "log.h"



bool nmt_module_load(NModule* module, const char* path) {
    *module = (NModule) {
        .path = NULL,
        .address = NULL,
        .fn_loaded = NULL,
        .fn_quit = NULL,
        .num_keybinds = 0,

        .events = {
            .fn_render   = NULL
        }
    };

    module->address = dlopen(path, RTLD_NOW);
    if(module->address == NULL) {
        logprintf(LOG_ERROR, "dlopen() failed for '%s' | %s", 
                path, strerror(errno));
        return false;
    }


    //dlerror(); // Clear any existing error.
    char* error = NULL;


   
    // Module load function is always needed.
    module->fn_loaded = dlsym(module->address, "module_loaded");
    if(module->fn_loaded == NULL) {
        error = dlerror();

        logprintf(LOG_ERROR, "Module '%s' needs implementation for 'module_loaded()'", path);
        if(error != NULL) {
            logprintf(LOG_ERROR, "dlerror(): %s", error);
        }

        dlclose(module->address);
        module->address = NULL;
        return false;
    }


    logprintf(LOG_INFO, "Module '%s' loaded", path, module->address);


    // Module quit function is optional, but very important if memory is needed to be freed.
    module->fn_quit = dlsym(module->address, "module_quit");
    
    // Events are optional.
    module->events.fn_render          = dlsym(module->address, "module_event_render");
    module->events.fn_char_input      = dlsym(module->address, "module_event_char_input");
    module->events.fn_key_input       = dlsym(module->address, "module_event_key_input");
    module->events.fn_window_resized  = dlsym(module->address, "module_event_window_resized");

    return true;
}

void nmt_module_quit(NModule* module) {
    if(!nmt_is_module_loaded(module)) {
        return;
    }

    if(module->fn_quit) {
        module->fn_quit();
    }

    dlclose(module->address);
    module->address = NULL;
}

bool nmt_is_module_loaded(NModule* module) {
    return (module != NULL && module->address != NULL);
}



