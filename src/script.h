#ifndef SCRIPT_H
#define SCRIPT_H

#include <EXTERN.h> // From perl
#include <perl.h>

#include <stdbool.h>


struct perl_script {
    PerlInterpreter* perl_interp;
    char filename [64];
    bool is_loaded;
};



struct nemi;

bool nemi_load_perl_script(struct nemi* st, const char* filepath);
void nemi_unload_perl_script(struct perl_script* script);

void plscript_call
    (struct perl_script* script, const char* func);

    // NOTE: 'args' must not be NULL
void plscript_call_args
    (struct perl_script* script, const char* func, char** args);

#endif
