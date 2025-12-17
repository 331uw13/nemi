#ifndef XS_WRAPPERS__NEMI_H
#define XS_WRAPPERS__NEMI_H

#include "EXTERN.h"
#include "XSUB.h"

#include "../plscript_funcs.h"

/*

   XSRETURN_IV    - Return int
   XSRETURN_NV    - Return double
   XSRETURN_PV    - Return string.
   XSRETURN_SV    - Return SV
   XSRETURN_UNDEF - Return 'undef'
   XSRETURN_EMPTY - Return void

*/
/*
XS(xsw_test_func) {
    dXSARGS;
    
    int a = SvIV(ST(0));
    int b = SvIV(ST(1));

    test_func(a, b);

    XSRETURN_EMPTY;
}
*/

XS(xsw_term_get_rows) {
    dXSARGS;
    (void)items;
    XSRETURN_IV(nemipl__term_get_rows());
}

XS(xsw_term_get_cols) {
    dXSARGS;
    (void)items;
    XSRETURN_IV(nemipl__term_get_cols());
}

XS(xsw_keydown) {
    dXSARGS;
    (void)items;
    int key = SvIV(ST(0));
    XSRETURN_IV(nemipl__keydown(key));
}

XS(xsw_term_ignore_keys) {
    dXSARGS;
    (void)items;
    nemipl__term_ignore_keys();
    XSRETURN_EMPTY;
}

XS(xsw_term_ignore_chars) {
    dXSARGS;
    (void)items;
    nemipl__term_ignore_chars();
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_keys) {
    dXSARGS; 
    (void)items;
    nemipl__term_unignore_keys();
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_chars) {
    dXSARGS;
    (void)items;
    nemipl__term_unignore_chars();
    XSRETURN_EMPTY;
}


#endif
