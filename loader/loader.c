#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


#include "../src/nemi.h"
#include "../src/string.h"
#include "../src/memory.h"



// If 'NEMI_ASSUME_IN_USERHOMEDIR' is 'true' then
// "/home/$USER/" is added before 'NEMI_HOMEDIR'.
#define NEMI_HOMEDIR ".nemi"
#define NEMI_ASSUME_IN_USERHOMEDIR true


struct nemi*(*nemi_start_session)(struct nemi_filepaths);
void(*nemi_quit_session)(struct nemi*);
void(*nemi_update_frame)(struct nemi*);
void(*nemi_create_msg)(struct nemi*st, const char*, ...);
void(*nemi_write_term)(struct terminal*, enum term_write_target, char* fmt, ...);
void(*nemi_prepare_from_hotreload)(struct nemi*);
void* libnemi;

static bool do_full_restart = false;

bool load_libnemi(const char* libpath) {
    libnemi = dlopen(libpath, RTLD_NOW);
    if(libnemi == NULL) {
        fprintf(stderr, "%s: dlopen(): %s\n", __func__, dlerror());
        return false;
    }

    nemi_start_session = dlsym(libnemi, "start_session");
    nemi_quit_session = dlsym(libnemi, "quit_session");
    nemi_update_frame = dlsym(libnemi, "update_frame");
    nemi_create_msg   = dlsym(libnemi, "create_msg");
    nemi_write_term   = dlsym(libnemi, "write_term");
    nemi_prepare_from_hotreload = dlsym(libnemi, "prepare_from_hotreload");
    // TODO: Add more error checking.

    return true;
}

bool run(struct nemi_filepaths filepaths) {
    load_libnemi(filepaths.libnemi);
  

    struct nemi* st = nemi_start_session(filepaths);
    if(!st) {
        return false;
    }

    st->flags |= FLG_RESTARTING_SUPPORTED;
    st->flags |= FLG_HOTRELOADING_SUPPORTED;

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
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

bool find_home(char** output) {
    bool is_found = false;
    struct string_t path = string_create(0);

    if(NEMI_ASSUME_IN_USERHOMEDIR) {
        char* env_home = getenv("HOME");
        if(env_home != NULL) {
            string_append(&path, env_home, -1);
        }
        else {
            string_append(&path, getpwuid(getuid())->pw_dir, -1);
        }
    }
    if(string_lastbyte(&path) != '/' && NEMI_HOMEDIR[0] != '/') {
        string_pushbyte(&path, '/');
    }
 

    string_append(&path, NEMI_HOMEDIR, -1);
    string_nullterm(&path);

    if(access(path.bytes, R_OK) == 0) {
        is_found = true;
        *output = strdup(path.bytes);
    }

    free_string(&path);
    return is_found;
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

    string_append(&path, nemi_homedir, -1);
    if(string_lastbyte(&path) != '/' && filename[0] != '/') {
        string_pushbyte(&path, '/');
    }
    string_append(&path, filename, -1);
    string_nullterm(&path);

    if(access(path.bytes, R_OK) == 0) {
        is_found = true;
        *output = strdup(path.bytes);
    }

    free_string(&path);
    return is_found;
}

int main(int argc, char** argv) {
    libnemi = NULL;
    
    struct nemi_filepaths filepaths;
    memset(&filepaths, 0, sizeof(filepaths));

    for(int i = 0; i < argc; i++) {
        bool last = i+1 >= argc;
        if(strcmp(argv[i], "-home") == 0 && !last) {
            filepaths.nemi_home = strdup(argv[i+1]);
            i++;
        }
    }

    if(filepaths.nemi_home == NULL) {
        if(!find_home(&filepaths.nemi_home)) {
            fprintf(stderr, "Could not find nemi home directory. Did you run install.sh?\n"
                    "If you dont want to install and just want to test, run: %s -home .\n",
                    argv[0]);
            goto out;
        }
    }

    if(!find_file(filepaths.nemi_home, &filepaths.libnemi, "libnemi.so")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.fonts, "fonts")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.configs, "configs")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.scripts, "scripts")) {
        goto out;
    }
    if(!find_file(filepaths.nemi_home, &filepaths.colorthemes, "configs/colorthemes")) {
        goto out;
    }

    printf("Home:    '%s'\n", filepaths.nemi_home);
    printf("Lib:     '%s'\n", filepaths.libnemi);
    printf("Fonts:   '%s'\n", filepaths.fonts);
    printf("Configs: '%s'\n", filepaths.configs);
    printf("Scripts: '%s'\n", filepaths.scripts);
    printf("Colors:  '%s'\n", filepaths.colorthemes);

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
    freeif(filepaths.colorthemes);
    return 0;
}
