#ifndef NEMI_PLSCRIPT_FUNCS_H
#define NEMI_PLSCRIPT_FUNCS_H



struct nemi;

void plscript_funcs_set_context(struct nemi* st);



//void test_func(int a, int b);


int nemipl__new_renderbuf(int size);
int nemipl__rb_add_rect(int rb_index, int x, int y, int w, int h, int color);
int nemipl__rb_add_text(int rb_index, int x, int y, char* text, size_t len, int color);
void nemipl__rb_update_rect(int rb_index, int rb_node_index, int x, int y, int w, int h, int color);
void nemipl__rb_update_text(int rb_index, int rb_node_inde, int x, int y, char* text, size_t len, int color);
void nemipl__rb_use_cellcoords(int rb_index);
void nemipl__rb_use_arbcoords(int rb_index);
void nemipl__term_scroll_y(int offset);
int nemipl__term_get_rows();
int nemipl__term_get_cols();
int nemipl__keydown(int key);
void nemipl__term_ignore_keys();
void nemipl__term_ignore_chars();
void nemipl__term_unignore_keys();
void nemipl__term_unignore_chars();


#endif
