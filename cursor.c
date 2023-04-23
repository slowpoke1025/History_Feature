#include <ncurses.h>
#include "cursor.h"

void move_x_delta(Cursor *cursor, int delta)
{
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, cursor->col + delta);
}
void move_x_pos(Cursor *cursor, int pos)
{
    getyx(stdscr, cursor->row, cursor->col); // get current cursor position，巨集展開賦值
    move(cursor->row, 7 + pos);
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
