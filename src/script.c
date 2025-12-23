
#include <unistd.h>


#include "script.h"
#include "nemi.h"
#include "common.h"
#include "string.h"
#include "xs_wrappers/nemi.h"


struct script_event_name {
    const char* name;
    int         num; // Should correspond to REG_EVENT... defined in "script.h"
};
static
const struct script_event_name SCRIPT_EVENTS[] = {
    { "event_key_input", REG_EVENT_KEY_INPUT },
    { "event_char_input", REG_EVENT_CHAR_INPUT },
    { "event_win_resized", REG_EVENT_WIN_RESIZED },
    { "event_term_buffer_changed", REG_EVENT_TERM_BUFFER_CHANGED }
};


const char* plscript_get_event_name(int event_num) {
    return SCRIPT_EVENTS[__builtin_ctz(event_num)].name;
}

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




static
void get_script_reg_events(struct perl_script* script, const char* script_filepath) {
    script->reg_events = 0;


    FILE* f = fopen(script_filepath, "r");
    if(!f) {
        logprintf(LOG_ERROR, "Failed to open \"%s\" | %s", 
                script_filepath, strerror(errno));
        return;
    }

    ssize_t line_len = 0;
    size_t line_memsize = 256;
    char* line = calloc(line_memsize, sizeof *line);

    const char* reg_event_tag = "#!REGISTER_EVENT\n";
    const size_t reg_event_taglen = strlen(reg_event_tag);

    int line_num = 0;

    // Search for "#!REGISTER_EVENT"
    // when found, check line after that to know what events
    // the script wants to get.
    while((line_len = getline(&line, &line_memsize, f)) != -1) {
        line_num++;
        if(line_len < (ssize_t)reg_event_taglen) {
            continue;
        }

        if(line[0] != '#' && line[1] != '!') {
            continue;
        }

        if(STR_MATCH(line, reg_event_tag)) {
            // Get next line.
            ssize_t line_len = getline(&line, &line_memsize, f);
            if(line_len < 0) {
                break;
            }

            if(string_charptr_find(line, line_len, "sub", 3)) {
                logprintf(LOG_ERROR, "Expected subroutine definition after %s Script \"%s\" line: %i",
                        reg_event_tag, script_filepath, line_num);
                continue;
            }

            for(size_t i = 0; i < ARRAY_LEN(SCRIPT_EVENTS); i++) {
                const struct script_event_name* event_name = &SCRIPT_EVENTS[i];
            
                if(string_charptr_find(
                            line, 
                            line_len, 
                            event_name->name,
                            strlen(event_name->name)) >= 0) {
                    script->reg_events |= event_name->num;
                    break;
                }
            }
        }
    }

    freeif(line);
    fclose(f);
}

bool load_perl_script(struct nemi* st, const char* filepath) {
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
    
    get_script_reg_events(script, filepath);

    logprintf(LOG_INFO, "Loaded script \"%s\"", filepath);

    register_functions(script);
    plscript_call(script, "init_script");    


    script->is_loaded = true;


    return true;
}


void unload_perl_script(struct perl_script* script) {
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


