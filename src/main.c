#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "nemi.h"





void active_terminal(struct terminal* term) {
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;


    const char* configs_dir = "./configs";
    struct nemi* st = start_session(configs_dir);
    if(!st) {
        return 1;
    }

    bool show_msgs = false;
    bool down = false;

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        begin_frame(st);

        read_terminal(st, st->terminal);
        render_terminal(st, st->terminal);
        update_terminal_blink_timer(st, st->terminal);
        end_frame(st);
        //printf("FrameTime = %f\n", st->frame_time * 1000.0);
    }

    quit_session(st);
    return 0;
}

