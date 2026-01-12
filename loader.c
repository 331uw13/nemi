#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>


#include "src/nemi.h"
#include "src/string.h"


struct nemi*(*nemi_start_session)(const char*);
void(*nemi_quit_session)(struct nemi*);
void(*nemi_update_frame)(struct nemi*);
void(*nemi_create_msg)(struct nemi*st, const char*, ...);
void* libnemi;

static bool do_hotreload = false;

bool load_libnemi() {
    libnemi = dlopen("./libnemi.so", RTLD_NOW);
    nemi_start_session = dlsym(libnemi, "start_session");
    nemi_quit_session = dlsym(libnemi, "quit_session");
    nemi_update_frame = dlsym(libnemi, "update_frame");
    nemi_create_msg   = dlsym(libnemi, "create_msg");
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
    string_append(&cmd, " && make -B -j", -1);
    string_append(&cmd, st->cfg.main.recompile_num_cores, -1);
    string_append(&cmd, ")", -1);

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

    pclose(pipe);
    return true;
}


void run() {
    const char* configs_dir = "./configs";
    struct nemi* st = NULL;

    load_libnemi();
    st = nemi_start_session(configs_dir);
    st->flags |= FLG_RECOMPILING_SUPPORTED;

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
        }
        nemi_update_frame(st);
    }

    close_libnemi(st);
}


int main(int argc, char** argv) {
reload:
    run();
    if(do_hotreload) {
        do_hotreload = false;
        printf("\033[1;32m Reloading...\033[0m\n");
        goto reload;
    }
    return 0;
}

