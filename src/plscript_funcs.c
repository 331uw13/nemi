#include <stdio.h>


#include "nemi.h"


static struct nemi* g_st = NULL;


void plscript_funcs_set_context(struct nemi* st) {
    g_st = st;
}


void test_func(int a, int b) {
    printf("C  %s: a=%i, b=%i\n", __func__, a, b);
}


int nemipl__new_renderbuf(int size) {
    return new_renderbuf(g_st, size);
}

int nemipl__rb_add_rect(int rb_index, int x, int y, int w, int h, int color) {
    return renderbuf_add_rect(g_st, &g_st->renderbufs[rb_index], x, y, w, h, color);
}
void nemipl__rb_update_rect(int rb_index, int rb_mesh_index, int x, int y, int w, int h, int color) {
    renderbuf_update_rect(g_st, &g_st->renderbufs[rb_index], rb_mesh_index, x, y, w, h, color);
}

int nemipl__rb_add_text(int rb_index, int x, int y, char* text, size_t len, int color) {
    printf("%s : %s\n",__func__, text);

    printf("%i\n", len);

    return 1234;
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

