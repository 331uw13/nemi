#ifndef NEMI_PLSCRIPT_FUNCS_H
#define NEMI_PLSCRIPT_FUNCS_H



struct nemi;

void plscript_funcs_set_context(struct nemi* st);



void test_func(int a, int b);
int nemipl__get_terminal_rows();
int nemipl__get_terminal_cols();
int nemipl__keydown(int key);



#endif
