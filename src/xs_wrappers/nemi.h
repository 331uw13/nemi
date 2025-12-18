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

XS(xsw_new_renderbuf) {
    dXSARGS;
    (void)items;
    int size = SvIV(ST(0));
    XSRETURN_IV(nemipl__new_renderbuf(size));
}

XS(xsw_rb_add_rect) {
    dXSARGS;
    (void)items;
    int rb_index= SvIV(ST(0));
    int x = SvIV(ST(1));
    int y = SvIV(ST(2));
    int w = SvIV(ST(3));
    int h = SvIV(ST(4));
    int color = SvIV(ST(5));
    XSRETURN_IV(nemipl__rb_add_rect(rb_index, x, y, w, h, color));
}

XS(xsw_rb_update_rect) {
    dXSARGS;
    (void)items;
    int rb_index= SvIV(ST(0));
    int rb_node_index= SvIV(ST(1));
    int x = SvIV(ST(2));
    int y = SvIV(ST(3));
    int w = SvIV(ST(4));
    int h = SvIV(ST(5));
    int color = SvIV(ST(6));
    nemipl__rb_update_rect(rb_index, rb_node_index, x, y, w, h, color);
    XSRETURN_EMPTY;
}

XS(xsw_rb_add_text) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int x = SvIV(ST(1));
    int y = SvIV(ST(2));
    STRLEN len;
    char*  str = SvPV(ST(3), len);
    int color = SvIV(ST(4));
    XSRETURN_IV(nemipl__rb_add_text(rb_index, x, y, str, len, color));
}

XS(xsw_rb_update_text) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));
    int x = SvIV(ST(2));
    int y = SvIV(ST(3));
    STRLEN len;
    char*  str = SvPV(ST(4), len);
    int color = SvIV(ST(5));
    nemipl__rb_update_text(rb_index, rb_node_index, x, y, str, len, color);
}

XS(xsw_rb_use_cellcoords) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    nemipl__rb_use_cellcoords(rb_index);
    XSRETURN_EMPTY;
}

XS(xsw_rb_use_arbcoords) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    nemipl__rb_use_arbcoords(rb_index);
    XSRETURN_EMPTY;
}

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
