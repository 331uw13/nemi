#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "nemi.h"

#include "tline.h"


void print_literal(const char* text, size_t len) {
   
    for(size_t i = 0; i < len; i++) {
        char ch = text[i];

        if(ch == 0x1B) { // ESC
            printf("\033[34m^[\033[0m");
            continue;
        }
        else
        if(ch == 0x07) { // BEL
            printf("\033[34m^G\033[0m");
            continue;
        }

        printf("%c", ch);
    }
    printf("\n");
    printf("\033[90m––––––––––––––––––-----------------------------------\033[0m\n");
}



int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    struct nemi* st = start_session();
    if(!st) {
        return 1;
    }


    char tmp_buffer[1024*4] = { 0 };

    while(!glfwWindowShouldClose(st->lfctx->glfw_win)) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if(st->last_char_in == 'r') {
            
            char* test = "ls -lh";
            execute_cmd(st->terminal, test, strlen(test));
        }

        size_t rd_bytes = 0;
        read_terminal(st->terminal, &rd_bytes, tmp_buffer, sizeof(tmp_buffer)-1);
        if(rd_bytes > 0) {
            print_literal(tmp_buffer, rd_bytes);
            push_terminal_line(st, st->terminal, tmp_buffer, rd_bytes);
        
            move_cursor_to_home(st->terminal);
        }

        render_terminal(st, st->terminal);


        end_frame(st);
    }

    quit_session(st);
    return 0;
}

