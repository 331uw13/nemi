#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../src/plscript_funcs.h"



MODULE = Nemi       PACKAGE = Nemi


int 
new_renderbuf()
    int size;
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__new_renderbuf(size);
    OUTPUT:
        RETVAL

void
rb_hide_node()
    int rb_index;
    int rb_node;
    PROTOTYPE: DISABLE
    CODE:
        

int
rb_add_rect()
    int rb_index;
    int x;
    int y;
    int w;
    int h;
    int color;
    PROTOTYPE: DISABLE
    CODE:
        RETVAL = nemipl__rb_add_rect(rb_index, x, y, w, h, color);
    OUTPUT:
        RETVAL

    
void
rb_update_rect()
    int rb_index;
    int rb_mesh_index;
    int x;
    int y;
    int w;
    int h;
    int color;
    CODE:
        nemipl__rb_update_rect(rb_index, rb_mesh_index, x, y, w, h, color);
    PROTOTYPE: DISABLE

int
rb_add_text()
    int rb_index;
    int x;
    int y;
    SV* str;
    int color;
    PROTOTYPE: DISABLE
    PREINIT:
        char* text;
        STRLEN text_len;
    CODE:
        text = SvPV(str, text_len);
        RETVAL = nemipl__rb_add_text(rb_index, x, y, text, text_len, color);
    OUTPUT:
        RETVAL

void
rb_update_text()
    int rb_index;
    int rb_node_index;
    int x;
    int y;
    SV* str;
    int color;
    PROTOTYPE: DISABLE
    PREINIT:
        char* text;
        STRLEN text_len;
    CODE:
        text = SvPV(str, text_len);
        nemipl__rb_update_text(rb_index, rb_node_index, x, y, text, text_len, color);


void
term_scroll_y()
    int offset;
    PROTOTYPE: DISABLE
    CODE:
        nemipl__term_scroll_y(offset);
        
void
rb_use_cellcoords()
    int rb_index;
    PROTOTYPE: DISABLE
    CODE:
        nemipl__rb_use_cellcoords(rb_index);

void
rb_use_arbcoords()
    int rb_index;
    PROTOTYPE: DISABLE
    CODE:
        nemipl__rb_use_arbcoords(rb_index);

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

