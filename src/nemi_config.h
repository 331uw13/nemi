#ifndef NEMI_CONFIG_H
#define NEMI_CONFIG_H




enum nemi_config_colors {
   
    NEMI_COLOR_BLACK,
    NEMI_COLOR_RED,
    NEMI_COLOR_GREEN,
    NEMI_COLOR_YELLOW,
    NEMI_COLOR_BLUE,
    NEMI_COLOR_MAGENTA,
    NEMI_COLOR_CYAN,
    NEMI_COLOR_WHITE,

    NEMI_BRIGHT_COLOR_BLACK,
    NEMI_BRIGHT_COLOR_RED,
    NEMI_BRIGHT_COLOR_GREEN,
    NEMI_BRIGHT_COLOR_YELLOW,
    NEMI_BRIGHT_COLOR_BLUE,
    NEMI_BRIGHT_COLOR_MAGENTA,
    NEMI_BRIGHT_COLOR_CYAN,
    NEMI_BRIGHT_COLOR_WHITE,
    
    NEMI_COLOR_BG,
    NEMI_COLOR_FG,
    
    NEMI_COLOR_MESSAGES_FG,
    NEMI_COLOR_MESSAGES_BG,
    NEMI_COLOR_MESSAGES_BORDER,
    NEMI_COLOR_COUNT
};




struct nemi;

struct nemi_config {
    char filepath [256];
    int padding_x;
    int padding_y;
    int line_padding;
    bool vsync;
    bool show_frametime;
    float italic_tilt;
    int   underline_height;
    float underline_offset;
    bool hide_mouse;
    bool soft_blink;
    float soft_blink_pow;
    float blink_speed;
    float char_spacing;
    struct color_t colors [NEMI_COLOR_COUNT];
};

struct nemi_font_config {
    char   font_filepath [256];
    bool   font_center_char_to_cell;
};

bool nemi_read_config(struct nemi* st, const char* filepath);
bool nemi_read_font_config(struct nemi* st, const char* filepath, struct nemi_font_config* font_cfg);





#endif
