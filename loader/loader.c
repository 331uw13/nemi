#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


#include "../src/nemi.h"
#include "../src/nmt_string.h"
#include "../src/memory.h"


static struct {
    void* lib;
    
    Nemi* (*start_session)(NemiFilepaths filepaths);
    void  (*quit_session)(Nemi* st);
    void  (*update_frame)(Nemi* st);
    
    // "leaf" is the graphics backend wrapper.
    // So we can use OpenGL, GLFW and freetype2 on desktop environments.
    // And if not needing desktop environment we can use linux framebuffer device.
    struct {
        bool (*should_quit)();
    }
    leaf;
}
nemi = {
    .lib = NULL,
    .start_session = NULL,
    .quit_session = NULL,
    .update_frame = NULL,
    .leaf.should_quit = NULL
};


static
bool load_libnemi(const char* libpath) {
    
    nemi.lib = dlopen(libpath, RTLD_NOW);
    if(nemi.lib == NULL) {
        fprintf(stderr, "%s: dlopen(): %s\n", __func__, dlerror());
        return false;
    }

    // TODO: Should add more error checking.

    nemi.start_session = dlsym(nemi.lib, "nmt_start_session");
    nemi.quit_session  = dlsym(nemi.lib, "nmt_quit_session");
    nemi.update_frame  = dlsym(nemi.lib, "nmt_update_frame");

    nemi.leaf.should_quit = dlsym(nemi.lib, "leaf_should_quit");

    return true;
    /*
    nemi_start_session = dlsym(libnemi, "nmt_start_session");
    nemi_quit_session = dlsym(libnemi,  "nmt_quit_session");
    nemi_update_frame = dlsym(libnemi,  "nmt_update_frame");
    nemi_create_msg   = dlsym(libnemi,  "nmt_create_msg");
    nemi_write_term   = dlsym(libnemi,  "write_term");
    nemi_prepare_from_hotreload = dlsym(libnemi, "prepare_from_hotreload");
    */
    
}


static
bool find_file
(
    char* nemi_homedir,
    char** output,
    char* filename
){
    bool is_found = false;
    struct string_t path = string_create(0);

    string_append(&path, nemi_homedir, strlen(nemi_homedir));
    if(string_lastbyte(&path) != '/' && filename[0] != '/') {
        string_pushbyte(&path, '/');
    }

    string_append(&path, filename, strlen(filename));
    string_nullterm(&path);

    if(access(path.bytes, R_OK) == 0) {
        is_found = true;
        *output = strdup(path.bytes);
    }
    else {
        fprintf(stderr, "Could not find configuration file '%s' from directory: '%s'\n",
                filename,
                nemi_homedir);
    }

    free_string(&path);
    return is_found;
}

int main(int argc, char** argv) {
    int exit_code = 0;
    nemi.lib = NULL;
    
    NemiFilepaths filepaths = { 0 };

    // TODO: Allow system wide installing...
    filepaths.nemi_home = strdup(".");

    if(!find_file(filepaths.nemi_home, &filepaths.libnemi, "libnemi.so")) {
        exit_code = 1;
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.fonts, "fonts")) {
        exit_code = 1;
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.configs, "configs")) {
        exit_code = 1;
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.modules, "modules")) {
        exit_code = 1;
        goto out;
    }

    printf("NemiFilepaths:\n");
    printf("Home:    '%s'\n", filepaths.nemi_home);
    printf("Lib:     '%s'\n", filepaths.libnemi);
    printf("Fonts:   '%s'\n", filepaths.fonts);
    printf("Configs: '%s'\n", filepaths.configs);
    printf("Modules: '%s'\n", filepaths.modules);
    printf("\n");


    if(!load_libnemi(filepaths.libnemi)) {
        return 1;
    }


    Nemi* st = nemi.start_session(filepaths);
    if(st == NULL) {
        return 1;
    }

    while(!nemi.leaf.should_quit()) {
        nemi.update_frame(st);
    }



    nemi.quit_session(st);
    dlclose(nemi.lib);
out:
    return exit_code;
}
/*

typedef struct Nemi_t Nemi;
typedef struct NemiFilepaths_t NemiFilepaths;
typedef struct NTerminal_t NTerminal;

Nemi*(*nemi_start_session)(NemiFilepaths filepaths);
void(*nemi_quit_session)(Nemi*);
void(*nemi_update_frame)(Nemi*);
void(*nemi_create_msg)(Nemi*st, const char*, ...);
void(*nemi_write_term)(NTerminal*, enum term_write_target, char* fmt, ...);
void(*nemi_prepare_from_hotreload)(Nemi*);
void* libnemi;


static bool do_full_restart = false;

bool load_libnemi(const char* libpath) {
    libnemi = dlopen(libpath, RTLD_NOW);
    if(libnemi == NULL) {
        fprintf(stderr, "%s: dlopen(): %s\n", __func__, dlerror());
        return false;
    }

    nemi_start_session = dlsym(libnemi, "nmt_start_session");
    nemi_quit_session = dlsym(libnemi,  "nmt_quit_session");
    nemi_update_frame = dlsym(libnemi,  "nmt_update_frame");
    nemi_create_msg   = dlsym(libnemi,  "nmt_create_msg");
    nemi_write_term   = dlsym(libnemi,  "write_term");
    nemi_prepare_from_hotreload = dlsym(libnemi, "prepare_from_hotreload");
    // TODO: Add error checking.

    return true;
}

bool run(NemiFilepaths filepaths) {
    load_libnemi(filepaths.libnemi);
  

    Nemi* st = nemi_start_session(filepaths);
    if(!st) {
        return false;
    }

    st->flags |= FLG_RESTARTING_SUPPORTED;
    st->flags |= FLG_HOTRELOADING_SUPPORTED;

    while(!leaf_should_quit()) {
        if(st->flags & FLG_LOADER_RESTART_SESSION) {
            printf("\033[1;32m Restarting...\033[0m\n");
            st->flags &= ~FLG_LOADER_RESTART_SESSION;
            do_full_restart = true;
            break;
        }

        if(st->flags & FLG_LOADER_HOTRELOAD_SESSION) {
            printf("\033[1;32m Hotreloading...\033[0m\n");
            st->flags &= ~FLG_LOADER_HOTRELOAD_SESSION;
            dlclose(libnemi);
            load_libnemi(filepaths.libnemi);

            // We need to set some global variables again after
            // hotreloading is done.
            nemi_prepare_from_hotreload(st);
        }

        nemi_update_frame(st);
    }

    nemi_quit_session(st);
    dlclose(libnemi);
    return true;
}

static
bool find_file
(
    char* nemi_homedir,
    char** output,
    char* filename
){
    bool is_found = false;
    struct string_t path = string_create(0);

    string_append(&path, nemi_homedir, strlen(nemi_homedir));
    if(string_lastbyte(&path) != '/' && filename[0] != '/') {
        string_pushbyte(&path, '/');
    }

    string_append(&path, filename, strlen(filename));
    string_nullterm(&path);

    if(access(path.bytes, R_OK) == 0) {
        is_found = true;
        *output = strdup(path.bytes);
    }
    else {
        fprintf(stderr, "Could not find configuration file '%s' from directory: '%s'\n",
                filename,
                nemi_homedir);
    }

    free_string(&path);
    return is_found;
}

int main(int argc, char** argv) {
    libnemi = NULL;
    
    NemiFilepaths filepaths = { 0 };
//    memset(&filepaths, 0, sizeof(filepaths));


    // TODO: Allow system wide installing...
    filepaths.nemi_home = strdup(".");

    if(!find_file(filepaths.nemi_home, &filepaths.libnemi, "libnemi.so")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.fonts, "fonts")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.configs, "configs")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.modules, "modules")) {
        goto out;
    }

    printf("NemiFilepaths:\n");
    printf("Home:    '%s'\n", filepaths.nemi_home);
    printf("Lib:     '%s'\n", filepaths.libnemi);
    printf("Fonts:   '%s'\n", filepaths.fonts);
    printf("Configs: '%s'\n", filepaths.configs);
    printf("Modules: '%s'\n", filepaths.modules);
    printf("\n");
    while(true) {
        if(!run(filepaths)) {
            break;
        }
   
        if(!do_full_restart) {
            break;
        }
       
        do_full_restart = false;
    } 

out:
    freeif(filepaths.nemi_home);
    freeif(filepaths.libnemi);
    freeif(filepaths.fonts);
    freeif(filepaths.configs);
    freeif(filepaths.modules);
    return 0;
}
*/
