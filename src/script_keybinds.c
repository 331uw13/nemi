#include <stdio.h>
#include <string.h>

#include "script_keybinds.h"
#include "nemi.h"
#include "log.h"
#include "common.h"


#define STB_DS_IMPLEMENTATION
#include "thirdparty/stb_ds.h"



struct keymap_elem {
    const char* str;
    const int  glfw_key;
};


static
const struct keymap_elem KEY_MAP[] = {
    { .str = "lshift", .glfw_key = GLFW_KEY_LEFT_SHIFT },
    { .str = "lctrl", .glfw_key = GLFW_KEY_LEFT_CONTROL },
    { .str = "lalt", .glfw_key = GLFW_KEY_LEFT_ALT },
    { .str = "lsuper", .glfw_key = GLFW_KEY_LEFT_SUPER }, 
    { .str = "rshift", .glfw_key = GLFW_KEY_RIGHT_SHIFT },
    { .str = "rctrl", .glfw_key = GLFW_KEY_RIGHT_CONTROL },
    { .str = "ralt", .glfw_key = GLFW_KEY_RIGHT_ALT },
    { .str = "rsuper", .glfw_key = GLFW_KEY_RIGHT_SUPER },
    { .str = "space", .glfw_key = GLFW_KEY_SPACE },
    { .str = "'", .glfw_key = GLFW_KEY_APOSTROPHE },
    { .str = ",", .glfw_key = GLFW_KEY_COMMA },
    { .str = "-", .glfw_key = GLFW_KEY_MINUS },
    { .str = ".", .glfw_key = GLFW_KEY_PERIOD },
    { .str = "/", .glfw_key = GLFW_KEY_SLASH },
    { .str = "0", .glfw_key = GLFW_KEY_0 },
    { .str = "1", .glfw_key = GLFW_KEY_1 },
    { .str = "2", .glfw_key = GLFW_KEY_2 },
    { .str = "3", .glfw_key = GLFW_KEY_3 },
    { .str = "4", .glfw_key = GLFW_KEY_4 },
    { .str = "5", .glfw_key = GLFW_KEY_5 },
    { .str = "6", .glfw_key = GLFW_KEY_6 },
    { .str = "7", .glfw_key = GLFW_KEY_7 },
    { .str = "8", .glfw_key = GLFW_KEY_8 },
    { .str = "9", .glfw_key = GLFW_KEY_9 },
    { .str = ";", .glfw_key = GLFW_KEY_SEMICOLON },
    { .str = "=", .glfw_key = GLFW_KEY_EQUAL },
    { .str = "a", .glfw_key = GLFW_KEY_A },
    { .str = "b", .glfw_key = GLFW_KEY_B },
    { .str = "c", .glfw_key = GLFW_KEY_C },
    { .str = "d", .glfw_key = GLFW_KEY_D },
    { .str = "e", .glfw_key = GLFW_KEY_E },
    { .str = "f", .glfw_key = GLFW_KEY_F },
    { .str = "g", .glfw_key = GLFW_KEY_G },
    { .str = "h", .glfw_key = GLFW_KEY_H },
    { .str = "i", .glfw_key = GLFW_KEY_I },
    { .str = "j", .glfw_key = GLFW_KEY_J },
    { .str = "k", .glfw_key = GLFW_KEY_K },
    { .str = "l", .glfw_key = GLFW_KEY_L },
    { .str = "m", .glfw_key = GLFW_KEY_M },
    { .str = "n", .glfw_key = GLFW_KEY_N },
    { .str = "o", .glfw_key = GLFW_KEY_O },
    { .str = "p", .glfw_key = GLFW_KEY_P },
    { .str = "q", .glfw_key = GLFW_KEY_Q },
    { .str = "r", .glfw_key = GLFW_KEY_R },
    { .str = "s", .glfw_key = GLFW_KEY_S },
    { .str = "t", .glfw_key = GLFW_KEY_T },
    { .str = "u", .glfw_key = GLFW_KEY_U },
    { .str = "v", .glfw_key = GLFW_KEY_V },
    { .str = "w", .glfw_key = GLFW_KEY_W },
    { .str = "x", .glfw_key = GLFW_KEY_X },
    { .str = "y", .glfw_key = GLFW_KEY_Y },
    { .str = "z", .glfw_key = GLFW_KEY_Z },
    { .str = "[", .glfw_key = GLFW_KEY_LEFT_BRACKET },
    { .str = "backslash", .glfw_key = GLFW_KEY_BACKSLASH },
    { .str = "]", .glfw_key = GLFW_KEY_RIGHT_BRACKET },
    { .str = "`", .glfw_key = GLFW_KEY_GRAVE_ACCENT },
    { .str = "esc", .glfw_key = GLFW_KEY_ESCAPE },
    { .str = "enter", .glfw_key = GLFW_KEY_ENTER },
    { .str = "tab", .glfw_key = GLFW_KEY_TAB },
    { .str = "backspace", .glfw_key = GLFW_KEY_BACKSPACE },
    { .str = "insert", .glfw_key = GLFW_KEY_INSERT },
    { .str = "delete", .glfw_key = GLFW_KEY_DELETE },
    { .str = "right", .glfw_key = GLFW_KEY_RIGHT },
    { .str = "left", .glfw_key = GLFW_KEY_LEFT },
    { .str = "down", .glfw_key = GLFW_KEY_DOWN },
    { .str = "up", .glfw_key = GLFW_KEY_UP },
    { .str = "pgup", .glfw_key = GLFW_KEY_PAGE_UP },
    { .str = "pgdn", .glfw_key = GLFW_KEY_PAGE_DOWN },
    { .str = "home", .glfw_key = GLFW_KEY_HOME },
    { .str = "end", .glfw_key = GLFW_KEY_END },
    { .str = "capslock", .glfw_key = GLFW_KEY_CAPS_LOCK},
    { .str = "scroll-lock", .glfw_key = GLFW_KEY_SCROLL_LOCK },
    { .str = "numlock", .glfw_key = GLFW_KEY_NUM_LOCK },
    { .str = "printscrn", .glfw_key = GLFW_KEY_PRINT_SCREEN },
    { .str = "pause", .glfw_key = GLFW_KEY_PAUSE },
    { .str = "f1", .glfw_key = GLFW_KEY_F1 },
    { .str = "f2", .glfw_key = GLFW_KEY_F2 },
    { .str = "f3", .glfw_key = GLFW_KEY_F3 },
    { .str = "f4", .glfw_key = GLFW_KEY_F4 },
    { .str = "f5", .glfw_key = GLFW_KEY_F5 },
    { .str = "f6", .glfw_key = GLFW_KEY_F6 },
    { .str = "f7", .glfw_key = GLFW_KEY_F7 },
    { .str = "f8", .glfw_key = GLFW_KEY_F8 },
    { .str = "f9", .glfw_key = GLFW_KEY_F9 },
    { .str = "f10", .glfw_key = GLFW_KEY_F10 },
    { .str = "f11", .glfw_key = GLFW_KEY_F11 },
    { .str = "f12", .glfw_key = GLFW_KEY_F12 }
};

//#define FNV_OFFSET 14695981039346656037UL
//#define FNV_PRIME 1099511628211UL

static
uint64_t hash_keys(int* keys, size_t num_keys) {
    /*
    uint64_t hash = FNV_OFFSET;
    int* k = keys;
    while(k && *k > 0) {
        hash ^= (uint64_t)(uint8_t)(*k);
        hash *= FNV_PRIME;
        k++;
    }
    return hash;
    */

    /*
    uint64_t hash = 7;
    int* k = keys;
    while(k && (*k >= 0)) {
        printf("%i\n", *k);
        hash = hash * 31 ^ *k;
        k++;
    }
    */

    uint64_t hash = 7;
    for(size_t i = 0; i < num_keys; i++) {
        int key = keys[i];
        hash = hash * 31 ^ key;
    }
    return hash;
}


// 'keys_str' is expected to be something like this. Example: "ctrl + a + b"
static
bool parse_keys_from_str(int* keys, size_t* num_keys_out, const char* keys_str, size_t keys_str_len) {
    
    const size_t key_str_max_len = 24;
    char   args [NEMI_SCRIPTS_KEYBIND_KEYS_MAX][key_str_max_len];
    size_t num_args = 0;
    size_t curr_arg_len = 0;

    char* ch = (char*)&keys_str[0];
    memset(args[0], 0, sizeof(args[0]));

    while(ch < keys_str + keys_str_len) {
        if(*ch == '+') {
            num_args++;
            curr_arg_len = 0;
            if(num_args >= NEMI_SCRIPTS_KEYBIND_KEYS_MAX) {
                num_args = NEMI_SCRIPTS_KEYBIND_KEYS_MAX;
                logprintf(LOG_WARN, 
                        "More than maximum keys in keybind str. Max is %li",
                        NEMI_SCRIPTS_KEYBIND_KEYS_MAX);
                break;
            }
            memset(args[num_args], 0, sizeof(args[num_args]));
            ch++;
            continue;
        }

        if(*ch == ' ') {
            ch++;
            continue;
        }

        args[num_args][curr_arg_len] = *ch; 
        curr_arg_len++;
        if(curr_arg_len >= key_str_max_len-1) {
            logprintf(LOG_ERROR, "Input '%s' has too long key name to be valid key",
                    keys_str);
            return false;
        }
        ch++;
    }
    num_args++;

    // Now we have separated the keys
    // but still need to figure out what do they correspond to.

    size_t num_keys = 0;
    for(size_t i = 0; i < num_args; i++) {
        const char* kstr = args[i];

        //printf("%s -> ", kstr);
        bool found_key = false;

        for(size_t j = 0; j < ARRAY_LEN(KEY_MAP); j++) {
            const struct keymap_elem* key = &KEY_MAP[j];
            if(STR_MATCH(kstr, key->str)) {
                keys[num_keys] = key->glfw_key;
                num_keys++;
                found_key = true;
                //printf("%i", key->glfw_key);
                break;
            }
        }

        if(!found_key) {
            logprintf(LOG_ERROR, "Didnt find corresponding key num for '%s', Input: \"%s\"",
                    kstr, keys_str);
            return false;
        }

        //printf("\n");
    }

    *num_keys_out = num_keys;
    return true;
}


void add_script_keybind
(
    Nemi* st,
    const char* script_name,
    const char* event_name,
    const char* keybind_str,
    size_t      keybind_str_len
){
    PerlScript* script = NULL;
    for(size_t i = 0; i < st->num_scripts; i++) {
        if(STR_MATCH(script_name, st->scripts[i].name)) {
            script = &st->scripts[i];
            break;
        }
    }

    if(script == NULL) {
        logprintf(LOG_ERROR, "Did not find script named \"%s\". Make sure the name matches one written in the config file.");
        return;
    }


    int* keys = calloc(NEMI_SCRIPTS_KEYBIND_KEYS_MAX+1, sizeof *keys);
    size_t num_keys = 0;
    if(!parse_keys_from_str(keys, &num_keys, keybind_str, keybind_str_len)) {
        logprintf(LOG_ERROR, "Failed to parse keys from '%s'", keybind_str);
        free(keys);
        return;
    }


    PerlScriptKeybindValue* value = malloc(sizeof *value);
    value->keys_str = strdup(keybind_str);
    value->event_name = strdup(event_name);

    hmput(script->keybind_map, hash_keys(keys, num_keys), value);
}

void handle_script_keybind_event(Nemi* st, PerlScript* script) {
    if(!(script->reg_events & REG_EVENT_KEYBIND_PRESS)) {
        return;
    }

    int keys [NEMI_SCRIPTS_KEYBIND_KEYS_MAX+1] = { 0 };
    size_t num_keys = 0;

    for(size_t i = 0; i < ARRAY_LEN(KEY_MAP); i++) {
        const struct keymap_elem* key = &KEY_MAP[i];
        
        if(glfwGetKey(st->lfctx->glfw_win, key->glfw_key) == GLFW_PRESS) {
            keys[num_keys] = key->glfw_key;
            num_keys++;

            if(num_keys >= NEMI_SCRIPTS_KEYBIND_KEYS_MAX) {
                break;
            }
        }
    }

    struct script_keybind* kb = hmgetp_null(script->keybind_map, hash_keys(keys, num_keys));
    if(kb == NULL) {
        return;
    }
    
    char* event_fn_args[] = {
        kb->value->event_name,
        NULL
    };

    plscript_call_args(script, "event_keybind_press", event_fn_args);
}
