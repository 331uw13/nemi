#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "tline.h"
#include "common.h"
#include "nemi.h"

#define LINE_DEF_MEMSIZE 64
#define LINE_REALLOC_BYTES 64


// Prepare memory for adding new data.
// Allocates more memory for line if needed.
static bool line_prep_mem_add(struct tline* line, uint32_t size_add) {
    if(line->chars && (line->num_chars + size_add < line->num_chars_alloc)) {
        return true;
    }

    size_t new_num_alloc = line->num_chars_alloc + size_add + LINE_REALLOC_BYTES;
    struct tchar* new_ptr = realloc(line->chars, new_num_alloc * sizeof *line->chars);
    
    if(!new_ptr) {
        fprintf(stderr, "%s:%i '%s()': %s\n",
                __FILE__,
                __LINE__,
                __func__,
                strerror(errno));
        return false;
    }

    line->chars = new_ptr;
    line->num_chars_alloc = new_num_alloc;
    line->num_newlines = 0;
    line->height = 0;

    //printf("%s -> %li\n", __func__, line->num_chars_alloc);
    return true;
}


struct tline create_tline() {
    struct tline line;
    line.num_chars = 0;
    line.chars = NULL;
    return line;
}

void free_tline(struct tline* line) {
    freeif(line->chars);
    line->num_chars = 0;
    line->num_chars_alloc = 0;
}

void tline_clear(struct tline* line) {
    if(!line->chars) {
        return;
    }
    memset(line->chars, 0, sizeof(*line->chars) * line->num_chars);
    line->num_chars = 0;
}



struct escape_seq_map_elem {
    int  num;
    bool attr_bg;
    int  attr;
};


static const struct escape_seq_map_elem ESCAPE_SEQ_MAP[] = {
    { .num = 0,  .attr_bg = false, .attr = ATTR_RESET },
    { .num = 1,  .attr_bg = false, .attr = ATTR_BOLD },
    { .num = 2,  .attr_bg = false, .attr = ATTR_DIM },
    { .num = 3,  .attr_bg = false, .attr = ATTR_ITALIC },
    { .num = 4,  .attr_bg = false, .attr = ATTR_UNDERLINE },
    { .num = 5,  .attr_bg = false, .attr = ATTR_BLINK },
    { .num = 6,  .attr_bg = false, .attr = ATTR_INVERSE },
    { .num = 7,  .attr_bg = false, .attr = ATTR_HIDDEN },
    { .num = 8,  .attr_bg = false, .attr = ATTR_STRIKETH },

    { .num = 30, .attr_bg = false, .attr = CHAR_COLOR_BLACK },
    { .num = 31, .attr_bg = false, .attr = CHAR_COLOR_RED },
    { .num = 32, .attr_bg = false, .attr = CHAR_COLOR_GREEN },
    { .num = 33, .attr_bg = false, .attr = CHAR_COLOR_YELLOW },
    { .num = 34, .attr_bg = false, .attr = CHAR_COLOR_BLUE },
    { .num = 35, .attr_bg = false, .attr = CHAR_COLOR_MAGENTA },
    { .num = 36, .attr_bg = false, .attr = CHAR_COLOR_CYAN },
    { .num = 37, .attr_bg = false, .attr = CHAR_COLOR_WHITE },
    { .num = 39, .attr_bg = false, .attr = CHAR_COLOR_DEFAULT },
    { .num = 90, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_BLACK },
    { .num = 91, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_RED },
    { .num = 92, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_GREEN },
    { .num = 93, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_YELLOW },
    { .num = 94, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_BLUE },
    { .num = 95, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_MAGENTA },
    { .num = 96, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_CYAN },
    { .num = 97, .attr_bg = false, .attr = CHAR_COLOR_BRIGHT_WHITE },
   
    { .num = 40, .attr_bg = true,  .attr = CHAR_COLOR_BLACK },
    { .num = 41, .attr_bg = true,  .attr = CHAR_COLOR_RED },
    { .num = 42, .attr_bg = true,  .attr = CHAR_COLOR_GREEN },
    { .num = 43, .attr_bg = true,  .attr = CHAR_COLOR_YELLOW },
    { .num = 44, .attr_bg = true,  .attr = CHAR_COLOR_BLUE },
    { .num = 45, .attr_bg = true,  .attr = CHAR_COLOR_MAGENTA },
    { .num = 46, .attr_bg = true,  .attr = CHAR_COLOR_CYAN },
    { .num = 47, .attr_bg = true,  .attr = CHAR_COLOR_WHITE },
    { .num = 39, .attr_bg = true,  .attr = CHAR_COLOR_DEFAULT },
    
};


void tline_add_ch(struct tline* line, struct tchar* ch) {
    if(!line_prep_mem_add(line, 1)) {
        return;
    }
    line->chars[line->num_chars++] = *ch;
}

void tline_add(struct nemi* st, struct tline* line, char* buffer, size_t size) {
    char* ch = &buffer[0];
   
    struct tchar es_ch;
    es_ch.ch = 0;
    es_ch.attr = ATTR_NONE;
    es_ch.attr_bg = false;
    es_ch.color = get_palette_color(st, CHAR_COLOR_DEFAULT);

    while(ch && ch < buffer+size) {

        if(*ch != 0x1B) {
            es_ch.ch = *ch;
            tline_add_ch(line, &es_ch);

            ch++;
            continue;
        }

        struct escape_seq es;
        ch = get_escape_seq_args(&es, buffer, ch, size);

        // Add character attributes if any is found.
        for(uint16_t i = 0; i < es.num_args; i++) {
            for(size_t j = 0; j < ARRAY_LEN(ESCAPE_SEQ_MAP); j++) {
                const struct escape_seq_map_elem* elem = &ESCAPE_SEQ_MAP[j];

                if(es.args[i] != elem->num) {
                    continue;
                }

                if(elem->attr >= CHAR_COLOR__BEGIN
                && elem->attr  < CHAR_COLOR__END) {
                    // Set color attribute.
                    es_ch.color = st->palette[ CHAR_COLOR__END - elem->attr ];
                }
                else {
                    if(elem->attr == ATTR_RESET) {
                        es_ch.attr = ATTR_NONE;
                        es_ch.color = get_palette_color(st, CHAR_COLOR_DEFAULT);
                    }
                    else {
                        // Add style attribute.
                        es_ch.attr |= elem->attr;
                    }
                }
                    
                es_ch.attr_bg = elem->attr_bg;
            }
        }
    }
}

int tline_render(struct nemi* st, struct tline* line, struct vec2i pos) {
    int num_newlines = 0;
    struct tchar* ch = &line->chars[0];
    while(ch < line->chars + line->num_chars) {

        if(ch->ch == '\n') {
            pos.y += st->font.char_height + st->line_padding_y;
            pos.x = 10;
            num_newlines++;
        }

        leaf_set_font_color(&st->font,
                (float)ch->color.red / 255.0f,
                (float)ch->color.grn / 255.0f,
                (float)ch->color.blu / 255.0f);
    
        pos.x += leaf_draw_char(&st->font, pos.x, pos.y, ch->ch);

        ch++;
    }

    return num_newlines;
}



