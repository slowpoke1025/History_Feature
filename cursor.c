#include <ncurses.h>
#include <string.h>
#include "cursor.h"

void move_x_delta(Cursor *cursor, int delta)
{
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, cursor->col + delta);
}
void move_x_pos(Cursor *cursor, char *base, int pos)
{
    if (!base)
        base = "";
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, strlen(base) + pos);
}

void move_x_end(Cursor *cursor)
{
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, COLS - 1);
}
void move_x_start(Cursor *cursor)
{
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, 0);
}

void clean_line(Cursor *cursor)
{
    move_x_start(cursor);
    clrtoeol();
}
