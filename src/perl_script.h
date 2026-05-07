#ifndef SCRIPT_H
#define SCRIPT_H

#include <EXTERN.h> // From perl
#include <perl.h>

#include <stdbool.h>
#include <stdint.h>


// Event nums.
#define REG_EVENT_HELP_MSG             (1 << 0)
#define REG_EVENT_KEY_INPUT            (1 << 1)
#define REG_EVENT_CHAR_INPUT           (1 << 2)
#define REG_EVENT_WIN_RESIZED          (1 << 3)
#define REG_EVENT_TERM_BUFFER_CHANGED  (1 << 4)
#define REG_EVENT_KEYBIND_PRESS        (1 << 5)
#define REG_EVENT_RENDER               (1 << 6)


typedef struct {
    char* event_name;  // Keybind event name.
    char* keys_str;
}
PerlScriptKeybindValue;

struct script_keybind {
    uint64_t key; // Hash from array of keys which require to trigger event name.
    PerlScriptKeybindValue* value;
};

typedef struct PerlScript_t {
    PerlInterpreter* perl_interp;
    char* name;
    char* filepath;
    bool is_loaded;
    int reg_events; // What events the script wants to get.
    //int uid; // Unique identitifer.

    struct script_keybind* keybind_map;
}
PerlScript;


typedef struct Nemi_t Nemi;


bool load_perl_script(Nemi* st, const char* filepath, const char* name);
void unload_perl_script(PerlScript* script);

// See above event numbers available, defined at top of this file.
const char* plscript_get_event_name(int event_num);

void plscript_call
    (PerlScript* script, const char* func);

    // NOTE: 'args' must not be NULL
void plscript_call_args
    (PerlScript* script, const char* func, char** args);

#endif
