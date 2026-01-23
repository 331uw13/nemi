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

/*
void close_libnemi(struct nemi* st) {
    nemi_quit_session(st);
    dlclose(libnemi);
}
*/

/*
bool recompile_src(struct nemi* st) {
    struct string_t cmd = string_create(0);
    string_append(&cmd, "(cd ", -1);
    string_append(&cmd, st->cfg.main.source_dir, -1);
    string_append(&cmd, " && make --output-sync=target -B -j", -1);
    string_append(&cmd, st->cfg.main.recompile_num_cores, -1);
    string_append(&cmd, " 2>&1)", -1);
    string_nullterm(&cmd);

    FILE* pipe = popen(cmd.bytes, "r");
    if(pipe == NULL) {
        nemi_create_msg(st, 
                "popen(\"%s\", \"r\") Failed! %s",
                cmd.bytes,
                strerror(errno)
        );
        return false;
    }
 
    char rd_buffer[512] = { 0 };
    while(fgets(rd_buffer, sizeof(rd_buffer)-1, pipe)) {
        nemi_create_msg(st, rd_buffer);
        nemi_update_frame(st);
    }

    int exit_code = WEXITSTATUS(pclose(pipe));
    return exit_code == 0;
}
*/
/*
struct arguments {
    char* nemi_homedir;
    char* libnemi_path;
    char* configs_dir;
    char* fonts_dir;
    char* startup_cmd;
    int   leaf_open_flags;
};
*/

bool run(struct nemi_filepaths filepaths) {
    load_libnemi(filepaths.libnemi);
  

    struct nemi* st = nemi_start_session(filepaths);
    if(!st) {
        return false;
    }


    /*
    struct nemi* st = nemi_start_session(
        args->nemi_homedir,
        args->fonts_dir,
        args->configs_dir,
        args->leaf_open_flags
    );
    if(!st) {
        return false;
    }
    */

    st->flags |= FLG_RESTARTING_SUPPORTED;
    st->flags |= FLG_HOTRELOADING_SUPPORTED;

    /*
    if(args->startup_cmd) {
        nemi_write_term(st->terminal, TERM_WRITE_PTY, args->startup_cmd);
        nemi_write_term(st->terminal, TERM_WRITE_PTY, "\n");
    }
    */

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
/*
void print_help() {
    printf(
        "Nemi - Terminal Emulator\n\n"
        " -exec <command>             Execute command at startup.\n"
        " -cfgdir <directory>         Specify configurations directory.\n"
        " -noresize                   Creates window which cannot be resized.\n"
        "\n"
        "Version: %s, build: %s\n"
        "Bug reports: https://github.com/331uw13/nemi  Thank you!:)\n",

        NEMI_VERSION_STR,
#ifdef DEVBUILD
        "(development)"
#else
        "(release)"
#endif
    );
}
*/
/*
struct arguments parse_arguments(int argc, char** argv) {
    struct arguments args = { 0 };
    for(int i = 0; i < argc; i++) {
        char* current_arg = argv[i];
        bool has_next = (i+1 < argc);
       
        if(strcmp(current_arg, "help") == 0
        || strcmp(current_arg, "-help") == 0
        || strcmp(current_arg, "--help") == 0) { 
            print_help();
        }
        else
        if(strcmp(current_arg, "-noresize") == 0) {
            args.leaf_open_flags |= LEAF_NORESIZE;
        }
        else
        if(strcmp(current_arg, "-exec") == 0 && has_next) {
            args.startup_cmd = strdup(argv[i+1]);
            i++; // Skip next.

        }
    }

    return args;
}
*/

/*

*/

/*
// Check first if the current directory has the library (for development purposes)
// If its not found then check '/home/$USER/.nemi/libnemi.so'
bool find_libnemi(struct arguments* args) {
    bool is_found = false;

    if(access("./libnemi.so", R_OK) == 0) {
        args->libnemi_path = strdup("./"LIBNEMI);
        is_found = true;
    }
    else {
        struct string_t path = string_create(0);
        string_append(&path, args->nemi_homedir, -1);
        
        if(string_lastbyte(&path) != '/' && LIBNEMI[0] != '/') {
            string_pushbyte(&path, '/');
        }
        string_append(&path, LIBNEMI, -1);
        string_nullterm(&path);

        if(access(path.bytes, R_OK) == 0) {
            args->libnemi_path = strdup(path.bytes);
            is_found = true;
        }
        free_string(&path);
    }

    return is_found;
}

// Check first if the current directory has configs dir (for development purposes)
// If its not found then check '/home/$USER/.nemi/configs'
bool find_configs_dir(struct arguments* args) {
    bool is_found = false;
 
    if(access("./configs", R_OK) == 0) {
        args->configs_dir = strdup("./configs");
        is_found = true;
    }
    else {
        struct string_t path = string_create(0);
        string_append(&path, args->nemi_homedir, -1);
        if(string_lastbyte(&path) != '/') {
            string_pushbyte(&path, '/');
        }
        string_append(&path, "configs", -1);
        string_nullterm(&path);

        if(access(path.bytes, R_OK) == 0) {
            args->configs_dir = strdup(path.bytes);
            is_found = true;
        }
    }
    return is_found;
}
*/

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


    if(!find_home(&filepaths.nemi_home)) {
        fprintf(stderr, "Could not find nemi home directory. '%s'\n", filepaths.nemi_home);
        goto out;
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
    if(!find_file(filepaths.nemi_home, &filepaths.colorthemes, "configs/colorthemes")) {
        goto out;
    }

    printf("Home:    '%s'\n", filepaths.nemi_home);
    printf("Lib:     '%s'\n", filepaths.libnemi);
    printf("Fonts:   '%s'\n", filepaths.fonts);
    printf("Configs: '%s'\n", filepaths.configs);
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
