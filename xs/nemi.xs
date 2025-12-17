#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../src/plscript_funcs.h"


MODULE = Nemi       PACKAGE = Nemi


void
test_func(a, b)
    int a
    int b
    PROTOTYPE: DISABLE


int
get_terminal_rows()
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__get_terminal_rows();
    OUTPUT:
        RETVAL

int
get_terminal_cols()
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__get_terminal_cols();
    OUTPUT:
        RETVAL

