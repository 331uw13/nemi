#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>

#include "module.h"
#include "log.h"
#include "nmt_string.h"



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


    module->current_dir = calloc(PATH_MAX, sizeof *module->current_dir);
    getcwd(module->current_dir, PATH_MAX);



    return true;
}

void nmt_module_quit(NModule* module) {
    if(!nmt_is_module_loaded(module)) {
        return;
    }

    if(module->fn_quit) {
        module->fn_quit();
    }

    if(module->current_dir) {
        free(module->current_dir);
    }

    dlclose(module->address);
    module->address = NULL;
}

bool nmt_is_module_loaded(NModule* module) {
    return (module != NULL && module->address != NULL);
}


bool nmt_module_chdir_rel(NModule* module, const char* relative_path) {
    bool ret_ok = false;
    if(relative_path == NULL) {
        ret_ok = true;
        goto err;
    }

    size_t rel_path_len = strlen(relative_path);
    if(rel_path_len == 0) {
        ret_ok = true;
        goto err;
    }


    struct string_t path = string_create(0);
    string_append(&path, module->current_dir, strlen(module->current_dir));


    char dirname[PATH_MAX] = { 0 };
    size_t dirname_len = 0;

    char* c = relative_path;
    char* rel_path_end = relative_path + rel_path_len;
   

    while(c < rel_path_end) {
        if(*c != '/') {
            if(dirname_len+1 >= sizeof(dirname)-1) {
                logprintf(LOG_ERROR, "'dirname' would overflow.");
                goto err_n_free;
            }
            dirname[ dirname_len++ ] = *c;
        }

        if(*c == '/' || (c + 1 >= rel_path_end)) {
         
            if(strcmp(dirname, "..") == 0) {
                // Jump back to parent dir.
    
                // Last character must be the fs root dir '/'
                if(path.size == 1) { 
                    break;
                }

                // Check last before entering loop, so it doesnt stop at the first iteration.
                if(string_lastbyte(&path) == '/') {
                    string_poplast(&path);
                }

                for(ssize_t i = path.size-1; i >= 0; i--) {
                    if(path.bytes[i] == '/') {
                        break;
                    }

                    string_poplast(&path);
                }
            }
            else {
                if(string_lastbyte(&path) != '/') {
                    string_pushbyte(&path, '/');
                }
                string_append(&path, dirname, dirname_len);
            }
            memset(dirname, 0, sizeof(dirname));
            dirname_len = 0;
        }

        c++;
    }
    

    string_nullterm(&path);
    ret_ok = nmt_module_chdir_abs(module, (const char*)path.bytes);

err_n_free:
    free_string(&path);
err:
    return ret_ok;
}

bool nmt_module_chdir_abs(NModule* module, const char* absolute_path) {
    if(access(absolute_path, F_OK) == 0) {
        if(module->current_dir != NULL) {
            free(module->current_dir);
        }

        module->current_dir = strdup(absolute_path);
        return true;
    }

    return false;
}

