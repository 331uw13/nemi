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

XS(xsw_draw_rect) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    int w = SvIV(ST(2));
    int h = SvIV(ST(3));
    int color = SvIV(ST(4));
    leaf_draw_rect(x, y, w, h, hexrgb_to_color_type(color));
}

XS(xsw_draw_rect_cells) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    int w = SvIV(ST(2));
    int h = SvIV(ST(3));
    int color = SvIV(ST(4));
    struct nemi* st = get_state();
    leaf_draw_rect(
            coltox(st, x),
            rowtoy(st, y),
            w * st->font.char_width,
            h * st->font.char_height,
            hexrgb_to_color_type(color));
}

XS(xsw_draw_text) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    STRLEN text_len;
    char* text = SvPV(ST(2), text_len);
    
    int color = SvIV(ST(3));
    
    struct nemi* st = get_state();
    struct color_t text_color = hexrgb_to_color_type(color);
    
    float old_font_r = st->font.char_color_r;
    float old_font_g = st->font.char_color_g;
    float old_font_b = st->font.char_color_b;
    st->font.char_color_r = (float)text_color.r / 255.0f;
    st->font.char_color_g = (float)text_color.g / 255.0f;
    st->font.char_color_b = (float)text_color.b / 255.0f;

    leaf_draw_text(&st->font, x, y, text, text_len);

    st->font.char_color_r = old_font_r;
    st->font.char_color_g = old_font_g;
    st->font.char_color_b = old_font_b;
}

XS(xsw_draw_text_cells) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    STRLEN text_len;
    char* text = SvPV(ST(2), text_len);
    
    int color = SvIV(ST(3));
    
    struct nemi* st = get_state();
    struct color_t text_color = hexrgb_to_color_type(color);
    
    float old_font_r = st->font.char_color_r;
    float old_font_g = st->font.char_color_g;
    float old_font_b = st->font.char_color_b;
    st->font.char_color_r = (float)text_color.r / 255.0f;
    st->font.char_color_g = (float)text_color.g / 255.0f;
    st->font.char_color_b = (float)text_color.b / 255.0f;

    leaf_draw_text(&st->font,
            coltox(st, x),
            rowtoy(st, y),
            text, text_len);

    st->font.char_color_r = old_font_r;
    st->font.char_color_g = old_font_g;
    st->font.char_color_b = old_font_b;
}

XS(xsw_term_get_yscroll_offset) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_IV(st->terminal->sb.offset);
}

XS(xsw_recompile) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    nemi_recompile_src(st);
    XSRETURN_EMPTY;
}

XS(xsw_add_keybind) {
    dXSARGS;
    (void)items;
    STRLEN script_name_len;
    char* script_name = SvPV(ST(0), script_name_len);
 
    STRLEN event_name_len;
    char* event_name = SvPV(ST(1), event_name_len);
   
    STRLEN keybind_str_len;
    char* keybind_str = SvPV(ST(2), keybind_str_len);

    struct nemi* st = get_state();
    add_script_keybind(st, 
            script_name,
            event_name,
            keybind_str,
            keybind_str_len);
}

XS(xsw_script_keybinds) { 
    dXSARGS;
    (void)items;
    STRLEN len;
    char*  script_name = SvPV(ST(0), len);
    struct nemi* st = get_state();
    nemi_message_script_keybinds(st, script_name);
}

XS(xsw_help) {
    dXSARGS;
    (void)items;
    STRLEN len;
    char*  str = SvPV(ST(0), len);
    struct nemi* st = get_state();
    nemi_help(st, str);
    XSRETURN_EMPTY;
}

XS(xsw_term_copy_to_clipboard) {
    dXSARGS;
    (void)items;
    int start_col = SvIV(ST(0));
    int start_row = SvIV(ST(1));
    int end_col = SvIV(ST(2));
    int end_row = SvIV(ST(3));
    STRLEN type_len;
    char* type = SvPV(ST(4), type_len);
    struct nemi* st = get_state();
    terminal_copy_to_clipboard(st, st->terminal, 
            start_col, start_row,
            end_col, end_row, type);
    XSRETURN_EMPTY;
}

XS(xsw_term_set_cell_custom_fg) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    int hex_rgb_color = SvIV(ST(2));
    struct nemi* st = get_state();
    terminal_set_cell_custom_fg(st->terminal, (VTermPos){ row, column }, hex_rgb_color);
    XSRETURN_EMPTY;
}

XS(xsw_term_set_cell_custom_bg) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    int hex_rgb_color = SvIV(ST(2));
    struct nemi* st = get_state();
    terminal_set_cell_custom_bg(st->terminal, (VTermPos){ row, column }, hex_rgb_color);
    XSRETURN_EMPTY;
}

XS(xsw_term_set_cell_custom_attrs) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    int attrs = SvIV(ST(2));
    struct nemi* st = get_state();
    terminal_set_cell_custom_attrs(st->terminal, (VTermPos){ row, column }, attrs);
    XSRETURN_EMPTY;
}

XS(xsw_term_clear_cell_custom_bg) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    struct nemi* st = get_state();
    terminal_clear_cell_custom_bg(st->terminal, (VTermPos){ row, column });
    XSRETURN_EMPTY;
}

XS(xsw_term_clear_cell_custom_fg) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    struct nemi* st = get_state();
    terminal_clear_cell_custom_fg(st->terminal, (VTermPos){ row, column });
    XSRETURN_EMPTY;
}

XS(xsw_term_clear_cell_custom_attrs) {
    dXSARGS;
    (void)items;
    int column = SvIV(ST(0));
    int row    = SvIV(ST(1));
    struct nemi* st = get_state();
    terminal_clear_cell_custom_attrs(st->terminal, (VTermPos){ row, column });
    XSRETURN_EMPTY;
}

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
    st->term_ignore_key_input_counter++;
    XSRETURN_EMPTY;
}

XS(xsw_term_ignore_chars) {
    dXSARGS;
    (void)items;
    
    struct nemi* st = get_state();
    st->term_ignore_char_input_counter++;
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_keys) {
    dXSARGS; 
    (void)items;
    
    struct nemi* st = get_state();
    st->term_ignore_key_input_counter--;
    if(st->term_ignore_key_input_counter < 0) {
        st->term_ignore_key_input_counter = 0;
    }
    XSRETURN_EMPTY;
}

XS(xsw_term_unignore_chars) {
    dXSARGS;
    (void)items;
    
    struct nemi* st = get_state();
    st->term_ignore_char_input_counter--;
    if(st->term_ignore_char_input_counter < 0) {
        st->term_ignore_char_input_counter = 0;
    }
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
