#ifndef XS_WRAPPERS__NEMI_H
#define XS_WRAPPERS__NEMI_H

#include "EXTERN.h"
#include "XSUB.h"

#include "nemi.h"

// https://perldoc.perl.org/perlguts


/*

   XSRETURN_IV    - Return int
   XSRETURN_NV    - Return double
   XSRETURN_PV    - Return string.
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

// TODO: Add error checking..

XS(xsw_list_scripts) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();

    if(st->num_scripts == 0) {
        create_msg(st, "No scripts loaded.");
        return;
    }

    for(size_t i = 0; i < st->num_scripts; i++) {
        struct perl_script* script = &st->scripts[i];
        create_msg(st, "%s %s %30s", 
                (script->is_loaded ? "\033[32m(Loaded)\033[0m" : "\033[31m(Failed to load)\033[0m"),
                script->name,
                script->filepath);
    }
}

XS(xsw_get_win_width) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();

    XSRETURN_IV(st->lfctx->win_width);
}
XS(xsw_get_win_height) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_IV(st->lfctx->win_height);
}

XS(xsw_get_font_charsize) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();

    mXPUSHi(st->font.char_width);
    mXPUSHi(st->font.char_height);
    XSRETURN(2);
}

XS(xsw_get_mouse_pos) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
   
    double p[2];
    glfwGetCursorPos(st->lfctx->glfw_win, &p[0], &p[1]);

    mXPUSHi((int)p[0]);
    mXPUSHi((int)p[1]);
    XSRETURN(2);
}

XS(xsw_get_mouse_cellpos) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
   
    double p[2];
    glfwGetCursorPos(st->lfctx->glfw_win, &p[0], &p[1]);

    mXPUSHi(floor((int)p[0] / st->font.char_width));
    mXPUSHi(floor((int)p[1] / (st->font.char_height + st->cfg.main.line_padding)));
    XSRETURN(2);
}

XS(xsw_term_exec) {
    dXSARGS;
    (void)items;
    STRLEN cmd_len;
    char* cmd = SvPV(ST(0), cmd_len);
    struct nemi* st = get_state();
 
    if(cmd_len == 0) {
        XSRETURN_EMPTY;
    }

    // Need to find some terminal to execute the command.
    // It cannot be echo terminal or one which is in altscreen mode.
    // TODO: This should be made to take in count if the terminal is already executing a command.
    struct terminal* term = NULL;
    for(uint16_t i = 0; i < st->num_terminals; i++) {
        struct terminal* candidate = &st->terminals[i];
        if(!candidate->is_altscreen && candidate->type == SHELL_TERMINAL) {
            term = candidate;
            break;
        }
    }

    if(term == NULL) {
        create_msg(st, "\033[31mFailed to find available terminal to execute command \"%s\"\033[0m",
                cmd);
        XSRETURN_EMPTY;
    }

    switch_terminal_ptr(st, term);
    write(term->master_fd, cmd, cmd_len);
    if(cmd[cmd_len-1] != '\n') {
        write(st->terminal->master_fd, "\n", 1);
    }
    XSRETURN_EMPTY;
}

XS(xsw_get_user_texteditor) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_PV(st->cfg.main.favourite_texteditor);
}

XS(xsw_draw_enable_scroll_offset) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    st->flags |= FLG_SCRIPTDRAW_ADJUSTPOS_TO_SCROLL;
}

XS(xsw_draw_disable_scroll_offset) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    st->flags &= ~FLG_SCRIPTDRAW_ADJUSTPOS_TO_SCROLL;
}

XS(xsw_restart) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    restart_session(st);
    XSRETURN_EMPTY;
}

XS(xsw_hotreload) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    hotreload_session(st);
    XSRETURN_EMPTY;
}

XS(xsw_draw_rect) {
    dXSARGS;
    (void)items;
    int x = SvIV(ST(0));
    int y = SvIV(ST(1));
    int w = SvIV(ST(2));
    int h = SvIV(ST(3));
    int color = SvIV(ST(4));
    struct nemi* st = get_state();
    leaf_draw_rect(x, y, w, h, hexrgb_to_color_type(color));
    XSRETURN_EMPTY;
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
            rowtoy(st, (st->flags & FLG_SCRIPTDRAW_ADJUSTPOS_TO_SCROLL) ? (y - st->terminal->yscroll) : y),
            w * st->font.char_width,
            h * st->font.char_height,
            hexrgb_to_color_type(color));
    XSRETURN_EMPTY;
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
    XSRETURN_EMPTY;
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
            rowtoy(st, (st->flags & FLG_SCRIPTDRAW_ADJUSTPOS_TO_SCROLL) ? (y - st->terminal->yscroll) : y),
            text, text_len);

    st->font.char_color_r = old_font_r;
    st->font.char_color_g = old_font_g;
    st->font.char_color_b = old_font_b;
    XSRETURN_EMPTY;
}

XS(xsw_term_get_yscroll) {
    dXSARGS;
    (void)items;
    struct nemi* st = get_state();
    XSRETURN_IV(st->terminal->yscroll);
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
    XSRETURN_EMPTY;
}

XS(xsw_script_keybinds) { 
    dXSARGS;
    (void)items;
    STRLEN len;
    char*  script_name = SvPV(ST(0), len);
    struct nemi* st = get_state();
    nemi_message_script_keybinds(st, script_name);
    XSRETURN_EMPTY;
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

XS(xsw_term_yscroll) {
    dXSARGS;
    (void)items;
    int offset = SvIV(ST(0));

    struct nemi* st = get_state();
    st->terminal->yscroll += offset;
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
    XSRETURN_EMPTY;
}



#endif
