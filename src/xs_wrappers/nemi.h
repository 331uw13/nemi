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

XS(xsw_get_terminal_rows) {
    dXSARGS;
    XSRETURN_IV(nemipl__get_terminal_rows());
}

XS(xsw_get_terminal_cols) {
    dXSARGS;
    XSRETURN_IV(nemipl__get_terminal_cols());
}

XS(xsw_keydown) {
    dXSARGS;
    int key = SvIV(ST(0));
    XSRETURN_IV(nemipl__keydown(key));
}



#endif
