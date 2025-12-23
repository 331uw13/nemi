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
        load_perl_script(v->st, value);
    }
    else
    if(STR_MATCH(section, "render_settings")) {
        if(STR_MATCH(name, "vsync")) {
            v->st->cfg.vsync = strbool_istrue(value);
        }
        else
        if(STR_MATCH(name, "padding_x")) {
            v->st->cfg.padding_x = atof(value);
        }
        else
        if(STR_MATCH(name, "padding_y")) {
            v->st->cfg.padding_y = atof(value);
        }
        else
        if(STR_MATCH(name, "line_padding")) {
            v->st->cfg.line_padding = atof(value);
        }
        else
        if(STR_MATCH(name, "italic_tilt")) {
            v->st->cfg.italic_tilt = atof(value);
        }
        else
        if(STR_MATCH(name, "underline_height")) {
            v->st->cfg.underline_height = atoi(value);
        }
        else
        if(STR_MATCH(name, "underline_offset")) {
            v->st->cfg.underline_offset = atof(value);
        }
    }
    else
    if(STR_MATCH(section, "misc_settings")) {
        if(STR_MATCH(name, "show_frametime")) {
            v->st->cfg.show_frametime = strbool_istrue(value);
        }
        else
        if(STR_MATCH(name, "hide_mouse")) {
            v->st->cfg.hide_mouse = strbool_istrue(value);
        }
    }



    return 1; // Continue reading.
}

static
int font_config_handler
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    if(!STR_MATCH(section, "font_settings")) { 
        return 1;
    }
   
    struct nemi_font_config* font_cfg = (struct nemi_font_config*)userptr;

    if(STR_MATCH(name, "font_filepath")) {
        const size_t value_len = strlen(value);
        if(value_len >= sizeof(font_cfg->font_filepath)-1) {
            logprintf(LOG_ERROR, "Font filepath is too long.");
            return 0;
        }

        strcpy(font_cfg->font_filepath, value);
    }
    else
    if(STR_MATCH(name, "font_center_char_to_cell")) {
        font_cfg->font_center_char_to_cell = strbool_istrue(value);
    }

    return 1;
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

bool nemi_read_font_config(struct nemi* st, const char* file, struct nemi_font_config* font_cfg) {
    if(ini_parse(file, font_config_handler, font_cfg)) {
        fprintf(stderr, "Cant parse config '%s', when trying to read font settings.\n", file);
        return false;
    }
    return true;
}

