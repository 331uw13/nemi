#include <stdio.h>
#include "nemi.h"

#include "tline.h"



int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    struct nemi* st = start_session();
    if(!st) {
        return 1;
    }


    struct tline line = create_tline();


    char* test = "\033[32mGreen text. \033[31mRed text. \033[0mReset.";
    tline_add(st, &line, test, strlen(test));


    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        end_frame(st);
    }


    free_tline(&line);

    quit_session(st);
    printf("\033[90mreturn 0\033[0m\n");
    return 0;
}

