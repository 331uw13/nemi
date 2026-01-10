#include <stdio.h>
#include "thirdparty/ini.h"

#include "nemi.h"
#include "nemi_config.h"
#include "common.h"
#include "string.h"


static
bool strtobool(const char* str) {
    return STR_MATCH(str, "1")
        || STR_MATCH(str, "true") 
        || STR_MATCH(str, "TRUE")
        || STR_MATCH(str, "True")
        || STR_MATCH(str, "yes")
        || STR_MATCH(str, "Yes")
        || STR_MATCH(str, "YES");
}



static
int config_handler__log_ini
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    struct log_settings* log_cfg = (struct log_settings*)userptr;

    if(STR_MATCH(name, "enabled")) {
        if(!strtobool(value)) {
            log_cfg->flags &= ~LOG_ENABLED;
        }
    }
    else
    if(STR_MATCH(name, "output")) {
        const size_t output_value_len = strlen(value);
        if(output_value_len >= sizeof(log_cfg->output)) {
            fprintf(stderr, "Config Error: log output value is too long.\n");
            return 0;
        }

        memset(log_cfg->output, 0, sizeof(log_cfg->output));
        memcpy(log_cfg->output, value, output_value_len);
    }
    else
    if(STR_MATCH(name, "use_color")) {
        if(strtobool(value)) {
            log_cfg->flags |= LOG_USE_COLOR;
        }
    }
    else
    if(STR_MATCH(name, "include_callee")) {
        if(strtobool(value)) {
            log_cfg->flags |= LOG_INCLUDE_CALLEE;
        }
    }
    else
    if(STR_MATCH(name, "enable_info")) {
        if(strtobool(value)) {
            log_cfg->flags |= LOG_INFO;
        }
    }
    else
    if(STR_MATCH(name, "enable_warnings")) {
        if(strtobool(value)) {
            log_cfg->flags |= LOG_WARN;
        }
    }
    else
    if(STR_MATCH(name, "enable_errors")) {
        if(strtobool(value)) {
            log_cfg->flags |= LOG_ERROR;
        }
    }
    else {
        fprintf(stderr, "No config entry named '%s' for %s", name, section);
    }
    
    return 1; // Continue reading.
}

static
int config_handler__font_ini
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    struct nemi_config* cfg = (struct nemi_config*)userptr;

    if(STR_MATCH(name, "filepath")) {
        size_t filepath_len = strlen(value);
        if(filepath_len >= sizeof(cfg->font.filepath)-1) {
            logprintf(LOG_ERROR, "Font filepath length is too long. Maximum = %li",
                    sizeof(cfg->font.filepath)-1);
            return 0;
        }

        memset(cfg->font.filepath, 0, sizeof(cfg->font.filepath));
        memmove(cfg->font.filepath, value, filepath_len);
    }
    else
    if(STR_MATCH(name, "center_char_to_cell")) {
        cfg->font.center_char_to_cell = strtobool(value);
    }
    else
    if(STR_MATCH(name, "char_spacing")) {
        cfg->font.char_spacing = atof(value);
    }
    else
    if(STR_MATCH(name, "italic_tilt")) {
        cfg->font.italic_tilt = atof(value);
    }
    else
    if(STR_MATCH(name, "underline_height")) {
        cfg->font.underline_height = atof(value);
    }
    else
    if(STR_MATCH(name, "underline_offset")) {
        cfg->font.underline_offset = atof(value);
    }
    else {
        logprintf(LOG_ERROR, "No config entry named '%s' for %s", name, section);
    }

    return 1; // Continue reading.
}

static
int config_handler__scripts_ini
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    struct nemi* st = (struct nemi*)userptr;
    load_perl_script(st, value, name);

    return 1; // Continue reading.
}

static
int config_handler__nemi_ini
(
    void* userptr,
    const char* section,
    const char* name,
    const char* value
){
    struct nemi_config* cfg = (struct nemi_config*)userptr;

    if(STR_MATCH(name, "padding_x")) {
        cfg->main.padding_x = atof(value);
    }
    else
    if(STR_MATCH(name, "padding_y")) {
        cfg->main.padding_y = atof(value);
    }
    else
    if(STR_MATCH(name, "line_padding")) {
        cfg->main.line_padding = atof(value);
    }
    else
    if(STR_MATCH(name, "vsync")) {
        cfg->main.vsync = strtobool(value);
    }
    else
    if(STR_MATCH(name, "soft_blink")) {
        cfg->main.soft_blink = strtobool(value);
    }
    else
    if(STR_MATCH(name, "soft_blink_pow")) {
        cfg->main.soft_blink_pow = atof(value);
    }
    else
    if(STR_MATCH(name, "blink_speed")) {
        cfg->main.blink_speed = atof(value);
    }
    else
    if(STR_MATCH(name, "show_frametime")) {
        cfg->main.show_frametime = strtobool(value);
    }
    else
    if(STR_MATCH(name, "hide_mouse")) {
        cfg->main.hide_mouse = strtobool(value);
    }
    else {
        logprintf(LOG_ERROR, "No config entry named '%s' for %s", name, section);
    }
    
    return 1; // Continue reading.
}


static
bool read_config
(
    int(*handler)(void*, const char*, const char*, const char*),
    void* userptr,
    char* configs_dir,
    char* config_file
){
    struct string_t path = string_create(0);

    string_append(&path, configs_dir, -1);
    if(string_lastbyte(&path) != '/') {
        string_pushbyte(&path, '/');
    }
    string_append(&path, config_file, -1);
    string_nullterm(&path);

    if(access(path.bytes, R_OK) != 0) {
        fprintf(stderr, "%s: '%s' No access for reading.\n", __func__, path.bytes);
        return false;
    }

    printf("%s: %s\n", __func__, path.bytes);

    if(ini_parse(path.bytes, handler, userptr)) {
        fprintf(stderr, "Cant parse config '%s'\n", path.bytes);
        return false;
    }

    free_string(&path);
    return true;
}

bool nemi_read_configs(struct nemi* st, const char* configs_dir) {

    struct log_settings log_settings = { 0 };
    log_settings.flags |= LOG_ENABLED;
    if(read_config(config_handler__log_ini, (void*)&log_settings, configs_dir, "log.ini")) {
        log_init(log_settings);
    }
    else {
        return false;
    }


    if(!read_config(config_handler__nemi_ini, &st->cfg, configs_dir, "nemi.ini")) {
        return false;
    }

    if(!read_config(config_handler__font_ini, &st->cfg, configs_dir, "font.ini")) {
        return false;
    }

    /*
    if(!read_config(config_handler__scripts_ini, st, configs_dir, "scripts.ini")) {
        return false;
    }
    */

    return true;
}

bool nemi_load_scripts(struct nemi* st, const char* configs_dir) {
    return read_config(config_handler__scripts_ini, st, configs_dir, "scripts.ini");
}
/*
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
        if(STR_MATCH(name, "char_spacing")) {
            v->st->cfg.char_spacing = atof(value);
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
        else
        if(STR_MATCH(name, "soft_blink")) {
            v->st->cfg.soft_blink = strbool_istrue(value);
        }
        else
        if(STR_MATCH(name, "soft_blink_pow")) {
            v->st->cfg.soft_blink_pow = atof(value);
        }
        else
        if(STR_MATCH(name, "blink_speed")) {
            v->st->cfg.blink_speed = atof(value);
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


bool nemi_read_config(struct nemi* st, const char* filepath) {
    struct config_values v = {
        .st = st,
        .log = { 0 }
    };

    v.log.flags = 0;
    v.log.flags |= LOG_ENABLED;

    v.pass = READ_LOG_SETTINGS;
    if(ini_parse(filepath, handler, &v)) {
        fprintf(stderr, "Cant parse config '%s'\n", filepath);
        return false;
    }
    
    log_init(v.log);

    v.pass = READ_OTHER_SETTINGS;
    if(ini_parse(filepath, handler, &v)) {
        fprintf(stderr, "Cant parse config '%s'\n", filepath);
        return false;
    }


    return true;
}

bool nemi_read_font_config(struct nemi* st, const char* filepath, struct nemi_font_config* font_cfg) {
    if(ini_parse(filepath, font_config_handler, font_cfg)) {
        fprintf(stderr, "Cant parse config '%s', when trying to read font settings.\n", filepath);
        return false;
    }
    return true;
}
*/
