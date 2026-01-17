#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>


#include "src/nemi.h"
#include "src/string.h"


struct nemi*(*nemi_start_session)(const char*, int);
void(*nemi_quit_session)(struct nemi*);
void(*nemi_update_frame)(struct nemi*);
void(*nemi_create_msg)(struct nemi*st, const char*, ...);
void(*nemi_write_term)(struct terminal*, enum term_write_target, char* fmt, ...);
void* libnemi;

static bool do_hotreload = false;

bool load_libnemi() {
    libnemi = dlopen("./libnemi.so", RTLD_NOW);
    nemi_start_session = dlsym(libnemi, "start_session");
    nemi_quit_session = dlsym(libnemi, "quit_session");
    nemi_update_frame = dlsym(libnemi, "update_frame");
    nemi_create_msg   = dlsym(libnemi, "create_msg");
    nemi_write_term   = dlsym(libnemi, "write_term");
    // TODO: Add error checking.

    return true;
}

void close_libnemi(struct nemi* st) {
    nemi_quit_session(st);
    dlclose(libnemi);
}


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

void print_help() {
    printf(
        "Nemi - Terminal Emulator arguments:\n\n"
        " -exec      :  Execute command at startup.\n"
        " -noresize  :  Creates window which cannot be resized.\n\n"
    );
}

// Returns index where 'arg' is found from 'argv'
// If its not found -1 is returned.
int argument_exists(int argc, char** argv, const char* arg) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], arg) == 0) {
            return i;
        }
    }
    return -1;
}

void run(int argc, char** argv) {
    const char* configs_dir = "./configs";
    char* startup_cmd = NULL;

    struct nemi* st = NULL;
    int leaf_open_flags = 0;


    if(argc > 1) {
        int arg_index = -1;

        if(argument_exists(argc, argv, "-help") > 0) {
            print_help();
        }

        if(argument_exists(argc, argv, "-noresize") > 0) {
            leaf_open_flags |= LEAF_NO_RESIZE;
        }
        
        if((arg_index = argument_exists(argc, argv, "-exec")) > 0) {
            if(arg_index+1 >= argc) {
                print_help();
            }
            else {
                startup_cmd = strdup(argv[arg_index+1]);
            }
        }
    }

    load_libnemi();
    st = nemi_start_session(configs_dir, leaf_open_flags);
    st->flags |= FLG_RECOMPILING_SUPPORTED;

    if(startup_cmd) {
        nemi_write_term(st->terminal, TERM_WRITE_PTY, startup_cmd);
        nemi_write_term(st->terminal, TERM_WRITE_PTY, "\n");
    }

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        if(glfwGetKey(st->lfctx->glfw_win, GLFW_KEY_LEFT_CONTROL)
        && glfwGetKey(st->lfctx->glfw_win, GLFW_KEY_T)) {
            do_hotreload = true;
            break;
        }

        if(st->flags & FLG_LOADER_SHOULD_RECOMPILE) {
            st->flags &= ~FLG_LOADER_SHOULD_RECOMPILE;
            if(recompile_src(st)) {
                do_hotreload = true;
                break;
            }
            else {
                nemi_create_msg(st, "\033[31mRecompiling failed.\033[0m");
            }
        }
        nemi_update_frame(st);
    }


    if(startup_cmd) {
        free(startup_cmd);
    }
    close_libnemi(st);
}


int main(int argc, char** argv) {
reload:
    run(argc, argv);
    if(do_hotreload) {
        do_hotreload = false;
        printf("\033[1;32m Reloading...\033[0m\n");
        goto reload;
    }
    return 0;
}

