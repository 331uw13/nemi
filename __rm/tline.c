#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "tline.h"
#include "common.h"
#include "nemi.h"
#include "terminal.h"

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
    

    return true;
}


struct tline create_tline() {
    struct tline line;
    line.chars = NULL;
    line.num_chars = 0;
    line.num_chars_alloc = 0;
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


void tline_add_ch_to_currln
(
    struct terminal* term,
    struct tchar* ch
){
    if(!term->currln) {
        term->currln = &term->lines[0];
    }
    if(!line_prep_mem_add(term->currln, 2)) {
        return;
    }
    term->currln = &term->lines[term->curs.pos.y];

    if(ch->ch == '\n') {
        // Ignore empty lines.
        if(term->currln->num_chars > 0) {
            
            terminal_prep_lines_add(term, 1);

            term->num_lines++;
            move_curs_to(term, 0, term->curs.pos.y + 1);
        }
        return;
    }

    // Support only ASCII characters for now.
    if(ch->ch >= 0x20 && ch->ch <= 0x7E) {
        //printf("--------------------------\n"); 
        term->curs.pos.x = clampi(term->curs.pos.x, 0, term->currln->num_chars);
        term->currln->chars[term->curs.pos.x] = *ch;
        term->currln->num_chars++;
        
        term->currln->chars[term->currln->num_chars].ch = '\0';

        term->num_added_tchars++;
        move_curs_off(term, 1, 0);
    }

    return;
}

void tline_add_buf_to_currln
(
    struct nemi* st,
    struct terminal* term,
    char* buffer,
    size_t size
){
    char* ch = &buffer[0];
   
    struct tchar es_ch;
    es_ch.ch = 0;
    es_ch.attr = ATTR_NONE;
    es_ch.attr_bg = false;
    es_ch.color = get_palette_color(st, CHAR_COLOR_DEFAULT);

    if(!term->currln) {
        term->currln = &term->lines[0];
    }

    while(ch && ch < buffer + size) {

        if(*ch == 0x08) {
            tline_backspace_currln(term);
        }

        if(*ch != 0x1B) {
            es_ch.ch = *ch;
            tline_add_ch_to_currln(term, &es_ch);
            ch++;
            continue;
        }
        else {
            char* ptr = terminal_handle_csi(st, term, ch, buffer, size);
            if(ptr) {
                ch = ptr;
                continue;
            }
        }
    
        ch++;



        // Parse color escape sequences.
        /*
        struct escape_seq es;
        ch = get_escape_seq_args(&es, buffer, ch, size);

        for(uint16_t i = 0; i < es.num_args; i++) {
            int arg = es.args[i];

            if(arg == OSC_ESCSEQ_BEGIN) {
                ch = handle_osc_escseq(st, ch, buffer, size);
                break;
            }

            for(size_t j = 0; j < ARRAY_LEN(ESCAPE_SEQ_MAP); j++) {
                const struct escape_seq_map_elem* elem = &ESCAPE_SEQ_MAP[j];

                if(arg != elem->num) {
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
        */
    }
}

void tline_backspace_currln(struct terminal* term) {
    if(!term->currln) {
        return;
    }

    if(term->currln->num_chars == 0) {
        return;
    }

    memset(&term->currln->chars[ term->currln->num_chars-1 ],
            0, sizeof(*term->currln));

    term->currln->num_chars--;

}

void tline_render(struct nemi* st, struct terminal* term, struct tline* line, int line_row) {
    struct tchar* ch = &line->chars[0];
    

    int pos_y = line_row + term->scroll.y;
    int pos_x = 10 + term->scroll.x;

    while(ch < line->chars + line->num_chars) {
        leaf_set_font_color(&st->font,
                (float)ch->color.red / 255.0f,
                (float)ch->color.grn / 255.0f,
                (float)ch->color.blu / 255.0f);
    
        pos_x += leaf_draw_char(&st->font, pos_x, pos_y, ch->ch);
        ch++;
    }
}


