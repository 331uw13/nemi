#ifndef XS_WRAPPERS__NEMI_H
#define XS_WRAPPERS__NEMI_H

#include "EXTERN.h"
#include "XSUB.h"

#include "nemi.h"


/*

   XSRETURN_IV    - Return int
   XSRETURN_NV    - Return double
   XSRETURN_PV    - Return string.
   XSRETURN_SV    - Return SV
   XSRETURN_UNDEF - Return 'undef'
   XSRETURN_EMPTY - Return void

*/

/*
 (xsw_test_func) {
    dXSARGS;
    
    int a = SvIV(ST(0));
    int b = SvIV(ST(1));

    test_func(a, b);

    XSRETURN_EMPTY;
}
*/

XS(xsw_term_get_char) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    struct nemi* st = get_state();

    XSRETURN_IV(terminal_get_char(st->terminal, column, row));
}

XS(xsw_term_get_cursor_x) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_IV(terminal_get_cursor_x(st->terminal));
}

XS(xsw_term_get_cursor_y) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_IV(terminal_get_cursor_y(st->terminal));
}

XS(xsw_new_renderbuf) {
    dXSARGS;
    (void)items;
    int num_nodes_max = SvIV(ST(0));
    XSRETURN_IV(new_renderbuf(get_state(), num_nodes_max));
}


XS(xsw_rb_remove_node) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));

    struct nemi* st = get_state();
    struct render_buffer* rb = &st->renderbufs[rb_index];
    renderbuf_remove_node(st, rb, rb_node_index);
}

XS(xsw_rb_hide_node) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));

    struct nemi* st = get_state();
    struct render_buffer* rb = &st->renderbufs[rb_index];
    rb->nodes[rb_node_index].hidden = true;
}


XS(xsw_rb_show_node) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));

    struct nemi* st = get_state();
    struct render_buffer* rb = &st->renderbufs[rb_index];
    rb->nodes[rb_node_index].hidden = false;
}

XS(xsw_rb_add_rect) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int x = SvIV(ST(1));
    int y = SvIV(ST(2));
    int w = SvIV(ST(3));
    int h = SvIV(ST(4));
    int color = SvIV(ST(5));
    struct nemi* st = get_state();
    XSRETURN_IV(renderbuf_add_rect(
                st,
                &st->renderbufs[rb_index],
                x, y, w, h,
                color));
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
    struct nemi* st = get_state();
    renderbuf_update_rect(st, &st->renderbufs[rb_index], rb_node_index, x, y, w, h, color);
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

    struct nemi* st = get_state();
    XSRETURN_IV(renderbuf_add_text(st, &st->renderbufs[rb_index], x, y, str, len, color));
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
    
    struct nemi* st = get_state();
    renderbuf_update_text(st, &st->renderbufs[rb_index], rb_node_index, x, y, str, len, color);
    XSRETURN_EMPTY;
}

XS(xsw_rb_node_layer_first) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));
    
    struct nemi* st = get_state();
    struct render_buffer* rb = &st->renderbufs[rb_index];

    rb->nodes[rb_node_index].layer = RBNODE_LAYER_FIRST;
    XSRETURN_EMPTY;
}

XS(xsw_rb_node_layer_last) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    int rb_node_index = SvIV(ST(1));
    
    struct nemi* st = get_state();
    struct render_buffer* rb = &st->renderbufs[rb_index];

    rb->nodes[rb_node_index].layer = RBNODE_LAYER_LAST;
    XSRETURN_EMPTY;
}

XS(xsw_term_hide_cells) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    int w = SvIV(ST(2));
    int h = SvIV(ST(3));

    struct nemi* st = get_state();
    terminal_hide_cells(st->terminal, true, x, y, w, h);
    XSRETURN_EMPTY;
}

XS(xsw_term_show_cells) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    int w = SvIV(ST(2));
    int h = SvIV(ST(3));

    struct nemi* st = get_state();
    terminal_hide_cells(st->terminal, false, x, y, w, h);
    XSRETURN_EMPTY;
}

XS(xsw_rb_use_cellcoords) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    
    struct nemi* st = get_state();
    st->renderbufs[rb_index].coordinate_mode = RBCOORDMODE_CELL;
    XSRETURN_EMPTY;
}

XS(xsw_rb_use_arbcoords) {
    dXSARGS;
    (void)items;
    int rb_index = SvIV(ST(0));
    
    struct nemi* st = get_state();
    st->renderbufs[rb_index].coordinate_mode = RBCOORDMODE_ARBITRARY;
    XSRETURN_EMPTY;
}

XS(xsw_term_scroll_y) {
    dXSARGS;
    (void)items;
    int offset = SvIV(ST(0));

    struct nemi* st = get_state();
    terminal_scroll(st->terminal, offset);
    XSRETURN_EMPTY;
}

XS(xsw_term_get_rows) {
    dXSARGS;
    (void)items;

    struct nemi* st = get_state();
    XSRETURN_IV(st->terminal->rows);
}

XS(xsw_term_get_cols) {
    dXSARGS;
    (void)items;
    
    struct nemi* st = get_state();
    XSRETURN_IV(st->terminal->cols);
}

XS(xsw_keydown) {
    dXSARGS;
    (void)items;
    int key = SvIV(ST(0));

    struct nemi* st = get_state();
    XSRETURN_IV(key_down(st, key));
}

XS(xsw_term_ignore_keys) {
    dXSARGS;
    (void)items;

    struct nemi* st = get_state();
    st->flags |= FLG_IGNORE_KEY_INPUT;
    XSRETURN_EMPTY;
}

XS(xsw_term_ignore_chars) {
    dXSARGS;
    (void)items;
    
    struct nemi* st = get_state();
    st->flags |= FLG_IGNORE_CHR_INPUT;
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_keys) {
    dXSARGS; 
    (void)items;
    
    struct nemi* st = get_state();
    st->flags &= ~FLG_IGNORE_KEY_INPUT;
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_chars) {
    dXSARGS;
    (void)items;
    
    struct nemi* st = get_state();
    st->flags &= ~FLG_IGNORE_CHR_INPUT;
    XSRETURN_EMPTY;
}

XS(xsw_create_msg) {
    dXSARGS;
    (void)items;
    
    STRLEN len;
    char*  str = SvPV(ST(0), len);
    struct nemi* st = get_state();

    create_msg(st, str);
}



#endif
