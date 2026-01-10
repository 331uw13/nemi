#ifndef SCRIPT_H
#define SCRIPT_H

#include <EXTERN.h> // From perl
#include <perl.h>

#include <stdbool.h>


// Event nums.
#define REG_EVENT_HELP_MSG             (1 << 0)
#define REG_EVENT_KEY_INPUT            (1 << 1)
#define REG_EVENT_CHAR_INPUT           (1 << 2)
#define REG_EVENT_WIN_RESIZED          (1 << 3)
#define REG_EVENT_TERM_BUFFER_CHANGED  (1 << 4)

struct perl_script {
    PerlInterpreter* perl_interp;
    char* name;
    bool is_loaded;
    int reg_events; // What events the script wants to get.
    int uid; // Unique identitifer.
};



struct nemi;

bool load_perl_script(struct nemi* st, const char* filepath, const char* name);
void unload_perl_script(struct perl_script* script);

const char* plscript_get_event_name(int event_num);

void plscript_call
    (struct perl_script* script, const char* func);

    // NOTE: 'args' must not be NULL
void plscript_call_args
    (struct perl_script* script, const char* func, char** args);

#endif
