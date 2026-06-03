#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "../../font.h"


// References:
// - https://aeb.win.tue.nl/linux/kbd/font-formats-1.html
// - https://wiki.osdev.org/PC_Screen_Font
// - https://www.zap.org.au/projects/console-fonts-utils/
bool leaf_load_font(LeafFont* font, const char* filepath) {

    gzFile file = gzopen(filepath, "r");
    if(file == NULL) {
        
    }

}

void leaf_unload_font(LeafFont* font) {
}
/*
void leaf_set_font_scale        (LeafFont* font, float scale);
void leaf_set_font_color        (LeafFont* font, RGBColor color);
void leaf_set_font_space_width  (LeafFont* font, float space_width);
void leaf_set_font_tab_width    (LeafFont* font, float tab_width);
void leaf_set_font_spacing      (LeafFont* font, float spacing);

void leaf_measure_text
(
    LeafFont* font, 
    int* width_out,
    int* height_out,
    const char* text,
    ssize_t text_length
);

void leaf_font_render(LeafFont* font);
*/
