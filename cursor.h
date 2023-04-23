#ifndef CURSOR_H
#define CURSOR_H
#define KEY_CTRL(c) ((c)&31)
typedef struct Cursor
{
    int row;
    int col;
} Cursor;

void move_x_delta(Cursor *cursor, int delta);
void move_x_pos(Cursor *cursor, int pos);
void move_x_end(Cursor *cursor);
void move_x_start(Cursor *cursor);
#endif