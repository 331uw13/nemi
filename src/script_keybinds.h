#ifndef SCRIPT_KEYBINDS_H
#define SCRIPT_KEYBINDS_H


typedef struct Nemi_t Nemi;
typedef struct PerlScript_t PerlScript;


void add_script_keybind
(
    Nemi* st,
    const char* script_name,
    const char* event_name,
    const char* keybind_str,
    size_t      keybind_str_len
);

void handle_script_keybind_event(Nemi* st, PerlScript* script);


#endif
