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




typedef struct Nemi_t Nemi;


struct nemi_config {
    
    struct {
        char* filepath;
        bool center_char_to_cell;
        float char_spacing;
        float italic_tilt;
        float underline_height;
        float underline_offset;
        float scale;
    }
    font;

    struct {
        int padding_x;
        int padding_y;
        int line_padding;
        bool vsync;
        bool soft_blink;
        float soft_blink_pow;
        float blink_speed;
        bool show_frametime;
        bool hide_mouse;
        char* source_dir;
        char* recompile_num_cores;
        char* favourite_texteditor;
        char* shell;
    }
    main;
   
    struct color_t colors [NEMI_COLOR_COUNT];
};

bool nmt_read_configs(Nemi* st, char* configs_dir);
void nmt_free_configs(Nemi* st);


#endif
