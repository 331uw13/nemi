#ifndef TERMINAL_LINE_H
#define TERMINAL_LINE_H


#include <stdint.h>
#include <sys/types.h>

#include "rgb_color.h"
#include "vec2.h"

#define ATTR_NONE       -1
#define ATTR_RESET      0

#define ATTR_BOLD       (1 << 0)
#define ATTR_DIM        (1 << 1)
#define ATTR_ITALIC     (1 << 2)
#define ATTR_UNDERLINE  (1 << 3)
#define ATTR_BLINK      (1 << 4)
#define ATTR_INVERSE    (1 << 5)
#define ATTR_HIDDEN     (1 << 6)
#define ATTR_STRIKETH   (1 << 7) // Strike through.

// Attributes and character colors must not overlap
// because their numbers are mapped to them. See "tline.c" for better info.
enum char_colors : int {
    CHAR_COLOR__BEGIN = (1 << 8),
    CHAR_COLOR_BLACK,
    CHAR_COLOR_RED,
    CHAR_COLOR_GREEN,
    CHAR_COLOR_YELLOW,
    CHAR_COLOR_BLUE,
    CHAR_COLOR_MAGENTA,
    CHAR_COLOR_CYAN,
    CHAR_COLOR_WHITE,
    CHAR_COLOR_BRIGHT_BLACK,
    CHAR_COLOR_BRIGHT_RED,
    CHAR_COLOR_BRIGHT_GREEN,
    CHAR_COLOR_BRIGHT_YELLOW,
    CHAR_COLOR_BRIGHT_BLUE,
    CHAR_COLOR_BRIGHT_MAGENTA,
    CHAR_COLOR_BRIGHT_CYAN,
    CHAR_COLOR_BRIGHT_WHITE,
    CHAR_COLOR_DEFAULT,

    CHAR_COLOR__END,
};

#define NUM_CHAR_COLORS (CHAR_COLOR__END - CHAR_COLOR__BEGIN)


struct tchar {
    char    ch;
    int     attr;
    bool    attr_bg; // Set to 'true' if 'attr' is meant for background.

    struct rgb_color color;
};

struct tline {
    struct tchar* chars;
    size_t        num_chars;
    size_t        num_chars_alloc;
};

struct nemi;
struct terminal;

struct tline create_tline();
void         free_tline(struct tline* line);
void         tline_clear(struct tline* line);

void tline_add_ch_to_currln  (struct terminal* term, struct tchar* ch);
void tline_add_buf_to_currln (struct nemi* st, struct terminal* term, char* buffer, size_t size);
void tline_backspace_currln  (struct terminal* term);

void tline_render (struct nemi* st, struct terminal* term, struct tline* line, int line_row);



#endif
