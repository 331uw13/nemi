#include "font.h"


void leaf_set_font_scale(LeafFont* font, float scale) {
    font->scale = scale;
    font->char_width = (font->real_char_width * scale) / 2;
    font->char_height = (font->real_char_height * scale) / 2;
   
    leaf_set_font_space_width(font, font->real_space_width);
    leaf_set_font_tab_width(font, font->real_tab_width);
}

void leaf_set_font_color(LeafFont* font, RGBColor color) {
#ifdef GRAPHICS_OPENGL
    font->char_color_r = (float)color.r / 255.0f;
    font->char_color_g = (float)color.g / 255.0f;
    font->char_color_b = (float)color.b / 255.0f;
#endif
#ifdef GRAPHICS_LINUX_FBDEV
    font->char_color = color;
#endif
}

void leaf_set_font_space_width(LeafFont* font, float space_width) {
    font->real_space_width = space_width;
    font->space_width = space_width * font->scale;
}

void leaf_set_font_tab_width(LeafFont* font, float tab_width) {
    font->real_tab_width = tab_width;
    font->tab_width = tab_width * font->scale;
}

void leaf_set_font_spacing(LeafFont* font, float spacing) {
    font->spacing = spacing * font->scale;
}
