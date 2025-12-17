#ifndef NEMI_PLSCRIPT_FUNCS_H
#define NEMI_PLSCRIPT_FUNCS_H



struct nemi;

void plscript_funcs_set_context(struct nemi* st);



//void test_func(int a, int b);
int nemipl__term_get_rows();
int nemipl__term_get_cols();
int nemipl__keydown(int key);
void nemipl__term_ignore_keys();
void nemipl__term_ignore_chars();
void nemipl__term_unignore_keys();
void nemipl__term_unignore_chars();


#endif
