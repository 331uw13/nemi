#include <stdio.h>


#include "nemi.h"


static struct nemi* g_st = NULL;


void plscript_funcs_set_context(struct nemi* st) {
    g_st = st;
    printf("%s\n",__func__);
}


void test_func(int a, int b) {
    printf("C  %s: a=%i, b=%i\n", __func__, a, b);
}


int nemipl__get_terminal_rows() {
    return g_st->terminal->rows;
}
int nemipl__get_terminal_cols() {
    return g_st->terminal->cols;
}
int nemipl__keydown(int key) {
    return (glfwGetKey(g_st->lfctx->glfw_win, key) == GLFW_PRESS);
}
