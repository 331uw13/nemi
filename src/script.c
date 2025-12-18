
#include <unistd.h>


#include "script.h"
#include "nemi.h"
#include "common.h"


#include "xs_wrappers/nemi.h"




static
struct perl_script* nemi_find_unloaded_script(struct nemi* st) {
    struct perl_script* ptr = NULL;

    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        if(!st->scripts[i].is_loaded) {
            ptr = &st->scripts[i];
        }
    }

    return ptr;
}



static
void register_functions(struct perl_script* script) {

    PerlInterpreter* my_perl = script->perl_interp;
    PERL_SET_CONTEXT(my_perl);

    newXS("Nemi::term_get_rows", xsw_term_get_rows, __FILE__);
    newXS("Nemi::term_get_cols", xsw_term_get_cols, __FILE__);
    newXS("Nemi::keydown", xsw_keydown, __FILE__);
    newXS("Nemi::term_ignore_chars",   xsw_term_ignore_chars, __FILE__);
    newXS("Nemi::term_ignore_keys",    xsw_term_ignore_keys, __FILE__);
    newXS("Nemi::term_unignore_chars", xsw_term_unignore_chars, __FILE__);
    newXS("Nemi::term_unignore_keys",  xsw_term_unignore_keys, __FILE__);
    newXS("Nemi::new_renderbuf",       xsw_new_renderbuf, __FILE__);
    newXS("Nemi::rb_add_rect",         xsw_rb_add_rect, __FILE__);
    newXS("Nemi::rb_update_rect",      xsw_rb_update_rect, __FILE__);
    newXS("Nemi::rb_add_text",         xsw_rb_add_text, __FILE__);
    newXS("Nemi::rb_update_text",      xsw_rb_update_text, __FILE__);
    newXS("Nemi::rb_use_cellcoords",   xsw_rb_use_cellcoords, __FILE__);
    newXS("Nemi::rb_use_arbcoords",    xsw_rb_use_arbcoords, __FILE__);
}


bool nemi_load_perl_script(struct nemi* st, const char* filepath) {
    if(!filepath) {
        return false;
    }

    if(access(filepath, R_OK) != 0) {
        logprintf(LOG_ERROR, "Cant read script \"%s\"", filepath);
        return false;
    }

    struct perl_script* script = nemi_find_unloaded_script(st);
    if(!script) {
        logprintf(LOG_ERROR, "No space to load new script, all slots have been taken.");
        return false;
    }



    script->perl_interp = perl_alloc();
    PERL_SET_CONTEXT(script->perl_interp);
    perl_construct(script->perl_interp);
    
    char* args[] = { "", filepath, NULL };
    perl_parse(script->perl_interp, NULL, 2, args, NULL);

    register_functions(script);
    plscript_call(script, "init_script");    


    script->is_loaded = true;
    logprintf(LOG_INFO, "Loaded script \"%s\"", filepath);


    return true;
}


void nemi_unload_perl_script(struct perl_script* script) {
    if(!script->is_loaded) {
        return;
    }

    PERL_SET_CONTEXT(script->perl_interp);
    perl_destruct(script->perl_interp);
    perl_free(script->perl_interp);
}

void plscript_call(struct perl_script* script, const char* func) {
    PerlInterpreter* my_perl = script->perl_interp;
    PERL_SET_CONTEXT(script->perl_interp);

    char* args[] = { NULL };

    call_argv(func, G_DISCARD | G_NOARGS, args);
}

void plscript_call_args(struct perl_script* script, const char* func, char** args) {
    PerlInterpreter* my_perl = script->perl_interp;
    PERL_SET_CONTEXT(script->perl_interp);

    call_argv(func, G_DISCARD, args); 
}


