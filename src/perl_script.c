
#include <unistd.h>


#include "perl_script.h"
#include "nemi.h"
#include "common.h"
#include "memory.h"
#include "string.h"
#include "nemi_xs_wrappers.h"
#include "thirdparty/stb_ds.h"


struct script_event_name {
    const char* name;
    int         num; // Should correspond to REG_EVENT... defined in "script.h"
};
static
const struct script_event_name SCRIPT_EVENTS[] = {
    { "event_help_message", REG_EVENT_HELP_MSG },
    { "event_key_input", REG_EVENT_KEY_INPUT },
    { "event_char_input", REG_EVENT_CHAR_INPUT },
    { "event_win_resized", REG_EVENT_WIN_RESIZED },
    { "event_term_buffer_changed", REG_EVENT_TERM_BUFFER_CHANGED },
    { "event_keybind_press", REG_EVENT_KEYBIND_PRESS },
    { "event_render", REG_EVENT_RENDER }
};


const char* plscript_get_event_name(int event_num) {
    return SCRIPT_EVENTS[__builtin_ctz(event_num)].name;
}

/*
static
PerlScript* nemi_find_unloaded_script(Nemi* st) {
    PerlScript* ptr = NULL;

    for(size_t i = 0; i < ARRAY_LEN(st->scripts); i++) {
        if(!st->scripts[i].is_loaded) {
            ptr = &st->scripts[i];
        }
    }

    return ptr;
}
*/



static
void register_functions(PerlScript* script) {

    PerlInterpreter* my_perl = script->perl_interp;
    PERL_SET_CONTEXT(my_perl);

#include "register_script_functions.inc"

}




static
void get_script_reg_events(PerlScript* script, const char* script_filepath) {
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
                            (char*)event_name->name,
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

bool load_perl_script(Nemi* st, const char* filepath, const char* name) {
    if(st->num_scripts+1 >= NEMI_SCRIPTS_MAX) {
        logprintf(LOG_ERROR, "Already loaded maximum amount of scripts.");
        return false;
    }

    if(!filepath) {
        return false;
    }

    if(access(filepath, R_OK) != 0) {
        logprintf(LOG_ERROR, "Cant read script \"%s\"", filepath);
        return false;
    }

    PerlScript* script = &st->scripts[st->num_scripts];
    st->num_scripts++;

    script->perl_interp = perl_alloc();
    PERL_SET_CONTEXT(script->perl_interp);
    perl_construct(script->perl_interp);
    
    char* args[] = { "", (char*)filepath, NULL };
    perl_parse(script->perl_interp, NULL, 2, args, NULL);
    
    get_script_reg_events(script, filepath);

    logprintf(LOG_INFO, "Loaded script \"%s\" %s", name, filepath);


    script->keybind_map = NULL;
    script->is_loaded = true;
    script->name = strdup(name);
    script->filepath = strdup(filepath);

    register_functions(script);
    plscript_call(script, "init_script");

    return true;
}


void unload_perl_script(PerlScript* script) {
    if(!script->is_loaded) {
        return;
    }

    if(script->keybind_map) {
        ptrdiff_t keybind_map_len = hmlen(script->keybind_map);
        for(ptrdiff_t i = 0; i < keybind_map_len; i++) {
            struct script_keybind* kb = &script->keybind_map[i];
            if(kb) {
                freeif(kb->value->keys_str);
                freeif(kb->value->event_name);
                free(kb->value);
            }
        }
    }

    PERL_SET_CONTEXT(script->perl_interp);
    perl_destruct(script->perl_interp);
    perl_free(script->perl_interp);
    freeif(script->name);
    freeif(script->filepath);
    script->is_loaded = false;
}

void plscript_call(PerlScript* script, const char* func) {
    void* old_context = PL_current_context; // Save old context if we are calling functions from scripts.

    PERL_SET_CONTEXT(script->perl_interp);

    char* args[] = { NULL };
    call_argv(func, G_DISCARD, args);
    
    PERL_SET_CONTEXT(old_context);
}

void plscript_call_args(PerlScript* script, const char* func, char** args) {
    void* old_context = PL_current_context;
    
    PERL_SET_CONTEXT(script->perl_interp);

    call_argv(func, G_DISCARD, args); 
    
    PERL_SET_CONTEXT(old_context);
}


