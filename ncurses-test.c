#include <stdio.h>
#include <ncurses.h>




int main() {
    initscr();
    raw();
    keypad(stdscr, 1);
    


    mvprintw(10, 10, "Cursor movement test.\n");
    char ch = getch();


    endwin();

    return 0;
}


