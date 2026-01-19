#include <stdio.h>
#include "thirdparty/ini.h"

#include "nemi.h"
#include "nemi_config.h"
#include "common.h"
#include "memory.h"
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
    (void)section;
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
    (void)section;
    struct nemi_config* cfg = (struct nemi_config*)userptr;

    if(STR_MATCH(name, "filepath")) {
        cfg->font.filepath = strdup(value);
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
    (void)section;
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
    (void)section;
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
    else
    if(STR_MATCH(name, "source_dir")) {
        cfg->main.source_dir = strdup(value);
    }
    else
    if(STR_MATCH(name, "recompile_num_cores")) {
        cfg->main.recompile_num_cores = strdup(value);
    }
    else
    if(STR_MATCH(name, "favourite_texteditor")) {
        cfg->main.favourite_texteditor = strdup(value);
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

    if(ini_parse(path.bytes, handler, userptr)) {
        fprintf(stderr, "Cant parse config '%s'\n", path.bytes);
        return false;
    }

    free_string(&path);
    return true;
}

bool nemi_read_configs(struct nemi* st, char* configs_dir) {

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

    return true;
}

bool nemi_load_scripts(struct nemi* st, char* configs_dir) {
    return read_config(config_handler__scripts_ini, st, configs_dir, "scripts.ini");
}

void free_configs(struct nemi* st) {
    freeif(st->cfg.main.source_dir);
    freeif(st->cfg.main.recompile_num_cores);
    freeif(st->cfg.main.favourite_texteditor);
    freeif(st->cfg.font.filepath);
}

