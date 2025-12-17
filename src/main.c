#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "nemi.h"






int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    struct nemi* st = start_session();
    if(!st) {
        return 1;
    }

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        begin_frame(st);
        
        read_terminal(st, st->terminal);
        render_terminal(st, st->terminal);

        end_frame(st);
    }

    quit_session(st);
    return 0;
}

