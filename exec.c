#define USERNAME_COLOR 1
#define COMMAND_COLOR 2
#define OUTPUT_COLOR 3
#define ERROR_COLOR 4
#include <ncurses.h>
int main()
{
    initscr(); // Initialize ncurses screen
    // 初始化颜色
    start_color();
    init_color(COLOR_MAGENTA, 219 * 1000 / 255, 112 * 1000 / 255, 147 * 1000 / 255);
    init_color(COLOR_CYAN, 0 * 1000 / 255, 255 * 1000 / 255, 255 * 1000 / 255);
    init_color(COLOR_GREEN, 0 * 1000 / 255, 255 * 1000 / 255, 0 * 1000 / 255);
    init_pair(USERNAME_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COMMAND_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(OUTPUT_COLOR, COLOR_GREEN, COLOR_BLACK);
    init_pair(ERROR_COLOR, COLOR_RED, COLOR_BLACK);

    // 设置文本颜色
    attrset(COLOR_PAIR(USERNAME_COLOR));
    printw("%s", "username");

    attrset(COLOR_PAIR(COMMAND_COLOR));
    printw("%s", "command");

    attrset(COLOR_PAIR(OUTPUT_COLOR));
    printw("%s", "output");

    attrset(COLOR_PAIR(ERROR_COLOR));
    printw("%s", "error");
    endwin();
    refresh();
}
