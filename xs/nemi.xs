#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../src/plscript_funcs.h"


MODULE = Nemi       PACKAGE = Nemi

void
term_ignore_keys()
    CODE:
        nemipl__term_ignore_keys();
    PROTOTYPE: DISABLE

void
term_ignore_chars()
    CODE:
        nemipl__term_ignore_chars();
    PROTOTYPE: DISABLE

void
term_unignore_keys()
    CODE:
        nemipl__term_unignore_keys();
    PROTOTYPE: DISABLE

void
term_unignore_chars()
    CODE:
        nemipl__term_unignore_chars();
    PROTOTYPE: DISABLE

int
term_get_rows()
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__term_get_rows();
    OUTPUT:
        RETVAL

int
term_get_cols()
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__term_get_cols();
    OUTPUT:
        RETVAL

