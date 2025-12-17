#include <stdio.h>


#include "nemi.h"


static struct nemi* g_st = NULL;


void plscript_funcs_set_context(struct nemi* st) {
    g_st = st;
}


void test_func(int a, int b) {
    printf("C  %s: a=%i, b=%i\n", __func__, a, b);
}


int nemipl__term_get_rows() {
    return g_st->terminal->rows;
}
int nemipl__term_get_cols() {
    return g_st->terminal->cols;
}
int nemipl__keydown(int key) {
    return (glfwGetKey(g_st->lfctx->glfw_win, key) == GLFW_PRESS);
}


void nemipl__term_ignore_keys() {
    g_st->flags |= FLG_IGNORE_KEY_INPUT;
}
void nemipl__term_ignore_chars() {
    g_st->flags |= FLG_IGNORE_CHR_INPUT;   
}
void nemipl__term_unignore_keys() {
    g_st->flags &= ~FLG_IGNORE_KEY_INPUT;
}
void nemipl__term_unignore_chars() {
    g_st->flags &= ~FLG_IGNORE_CHR_INPUT;
}

