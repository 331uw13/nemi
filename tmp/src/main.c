

#include <stdio.h>
#include <string.h>
#include <vterm.h>







int main() {

    int rows = 30;
    int cols = 30;

    VTerm* term = vterm_new(rows, cols);
    vterm_set_utf8(term, 1);

    VTermScreen* scrn = vterm_obtain_screen(term);
    vterm_screen_enable_altscreen(scrn, 1);
    vterm_screen_reset(scrn, 1);


    const char* test_input = "asdasdasdasdass\n";


    vterm_input_write(term, test_input, strlen(test_input));
    vterm_screen_flush_damage(scrn);


    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {

            VTermScreenCell cell;
            if(!vterm_screen_get_cell(scrn, (VTermPos){ r , c }, &cell)) {
                continue;
            }

            for(int i = 0; i < cell.width; i++) {
                printf("%c", (char)cell.chars[i]);
            }
        }
    }

    printf("\n");
    
    vterm_free(term);


    return 0;
}
