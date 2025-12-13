#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "nemi.h"

#include "tline.h"


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    struct nemi* st = start_session();
    if(!st) {
        return 1;
    }

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        glClearColor(0.045f, 0.045f, 0.045f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if(st->last_key_in == GLFW_KEY_ESCAPE) {
            break;
        }

        read_terminal(st, st->terminal);
        render_terminal(st, st->terminal);

        end_frame(st);
    }

    quit_session(st);
    return 0;
}

