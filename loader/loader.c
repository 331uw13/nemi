#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>



#include "../src/nemi.h"
#include "../src/string.h"
#include "../src/memory.h"


struct nemi*(*nemi_start_session)(const char*, int);
void(*nemi_quit_session)(struct nemi*);
void(*nemi_update_frame)(struct nemi*);
void(*nemi_create_msg)(struct nemi*st, const char*, ...);
void(*nemi_write_term)(struct terminal*, enum term_write_target, char* fmt, ...);
void(*nemi_prepare_from_hotreload)(struct nemi*);
void* libnemi;

static bool do_full_restart = false;

bool load_libnemi() {
    libnemi = dlopen("./libnemi.so", RTLD_NOW);
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

struct arguments {
    char* configs_dir;
    char* startup_cmd;
    int   leaf_open_flags;
};

void run(struct arguments* args) {
    load_libnemi();
    struct nemi* st = nemi_start_session(args->configs_dir, args->leaf_open_flags);

    st->flags |= FLG_RESTARTING_SUPPORTED;
    st->flags |= FLG_HOTRELOADING_SUPPORTED;

    if(args->startup_cmd) {
        nemi_write_term(st->terminal, TERM_WRITE_PTY, args->startup_cmd);
        nemi_write_term(st->terminal, TERM_WRITE_PTY, "\n");
    }

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
            load_libnemi();

            // We need to set some global variables again after
            // hotreloading is done.
            nemi_prepare_from_hotreload(st);
        }

        nemi_update_frame(st);
    }

    nemi_quit_session(st);
    dlclose(libnemi);
}

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


int main(int argc, char** argv) {
    libnemi = NULL;
    struct arguments args = parse_arguments(argc, argv);
    if(args.configs_dir == NULL) {
        args.configs_dir = strdup("./configs");
    }

restart:
    run(&args);
    if(do_full_restart) {
        do_full_restart = false;
        goto restart;
    }

    freeif(args.configs_dir);
    freeif(args.startup_cmd);
    
    return 0;
}
