#ifndef SCRIPT_KEYBINDS_H
#define SCRIPT_KEYBINDS_H


struct nemi;
struct perl_script;


void add_script_keybind
(
    struct nemi* st,
    const char* script_name,
    const char* event_name,
    const char* keybind_str,
    size_t      keybind_str_len
);

void handle_script_keybind_event(struct nemi* st, struct perl_script* script);


#endif
