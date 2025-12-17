#include <stdio.h>

#include "thirdparty/ini.h"

#include "nemi.h"
#include "nemi_config.h"
#include "common.h"



enum config_read_pass {
    READ_LOG_SETTINGS,
    READ_OTHER_SETTINGS
};

struct config_values {
    enum config_read_pass pass;
    struct nemi* st;
    struct log_settings log;
};


static
bool strbool_istrue(const char* str) {
    return STR_MATCH(str, "true");
}

static
int handler
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    struct config_values* v = (struct config_values*)userptr;

    if(v->pass == READ_LOG_SETTINGS) { 
        if(!STR_MATCH(section, "log_settings")) {
            return 1;
        }

        if(STR_MATCH(name, "enabled")) {
            if(!strbool_istrue(value)) {
                v->log.flags &= ~LOG_ENABLED;
            }
        }
        else
        if(STR_MATCH(name, "output")) {
            const size_t output_value_len = strlen(value);
            if(output_value_len >= sizeof(v->log.output)) {
                fprintf(stderr, "Config Error: log output value is too long.\n");
                return 0;
            }

            memset(v->log.output, 0, sizeof(v->log.output));
            memcpy(v->log.output, value, output_value_len);
        }
        else
        if(STR_MATCH(name, "use_color")) {
            if(strbool_istrue(value)) {
                v->log.flags |= LOG_USE_COLOR;
            }
        }
        else
        if(STR_MATCH(name, "include_callee")) {
            if(strbool_istrue(value)) {
                v->log.flags |= LOG_INCLUDE_CALLEE;
            }
        }
        else
        if(STR_MATCH(name, "enable_info")) {
            if(strbool_istrue(value)) {
                v->log.flags |= LOG_INFO;
            }
        }
        else
        if(STR_MATCH(name, "enable_warnings")) {
            if(strbool_istrue(value)) {
                v->log.flags |= LOG_WARN;
            }
        }
        else
        if(STR_MATCH(name, "enable_errors")) {
            if(strbool_istrue(value)) {
                v->log.flags |= LOG_ERROR;
            }
        }
        
        return 1; // Continue reading.
    }

    if(STR_MATCH(section, "scripts")) {
        nemi_load_perl_script(v->st, value);
    }



    return 1; // Continue reading.
}


bool nemi_read_config(struct nemi* st, const char* file) {
    printf("config = '%s'\n", file);
   
    struct config_values v = {
        .st = st,
        .log = { 0 }
    };

    v.log.flags = 0;
    v.log.flags |= LOG_ENABLED;

    v.pass = READ_LOG_SETTINGS;
    if(ini_parse(file, handler, &v)) {
        fprintf(stderr, "Cant parse config '%s'\n", file);
        return false;
    }
    
    log_init(v.log);

    v.pass = READ_OTHER_SETTINGS;
    if(ini_parse(file, handler, &v)) {
        fprintf(stderr, "Cant parse config '%s'\n", file);
        return false;
    }


    return true;
}
