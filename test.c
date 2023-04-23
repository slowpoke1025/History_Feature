
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "cursor.h"
#include "history.h"

// gcc test.c -o a -l ncurses

typedef struct CommandLine
{
    Stack left;
    RStack right;
    char *history;
} CommandLine;

void init_cli(CommandLine *CLI, char *history)
{
    strcpy(CLI->left.value, history);

    CLI->left.top = strlen(history) - 1;
    CLI->right.top = BUFFER_SIZE;
    CLI->history = history;
}

int empty_cli(CommandLine *CLI)
{
    return empty_stack(&(CLI->left)) && empty_rstack(&(CLI->right));
}

void clean_line(Cursor *cursor)
{
    move_x_start(cursor);
    clrtoeol();
}
void init_window()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
}

int main()
{
    init_window();

    History *history = init_history(NULL);

    CommandLine CLI;
    Stack *left = &(CLI.left);
    RStack *right = &(CLI.right);
    init_stack(left);
    init_rstack(right);
    Cursor cursor;

    printw("Enter> ");

    while (1)
    {
        int ch = getch();

        switch (ch)
        {
        case KEY_UP:

            clean_line(&cursor);
            close_stack(left);
            sprintf(history->buffer[history->tmp_count], "%s%s", left->value, right->value + right->top);

            char *prev_commond;
            if (history->tmp_count == 0)
            {
                printw("No prev command!\n");
                prev_commond = history->buffer[history->tmp_count];
            }
            else
            {
                prev_commond = history->buffer[--history->tmp_count];
            }

            init_cli(&CLI, prev_commond);
            printw("Enter> %s", prev_commond);
            break;
        case KEY_CTRL('d'):

            exit(0);

            break;
        case KEY_DOWN:

            clean_line(&cursor);
            close_stack(left);
            sprintf(history->buffer[history->tmp_count], "%s%s", left->value, right->value + right->top);

            char *next_command;

            if (history->tmp_count == history->count)
            {
                next_command = history->buffer[history->tmp_count];
                printw("No new command!\n");
            }
            else
            {
                next_command = history->buffer[++history->tmp_count];
            }
            init_cli(&CLI, next_command);
            printw("Enter> %s", next_command);
            break;

        case KEY_LEFT:
            if (!empty_stack(left))
            {
                push_rstack(right, pop_stack(left));
                move_x_delta(&cursor, -1);
            }
            break;

        case KEY_RIGHT:
            if (!empty_rstack(right))
            {
                push_stack(left, pop_rstack(right));
                move_x_delta(&cursor, 1);
            }
            break;

        case '\n':
            if (!empty_cli(&CLI))
            {
                close_stack(left);
                sprintf(history->buffer[history->count], "%s%s", left->value, right->value + right->top);
                move_x_end(&cursor);
                printw("\n%d %s\n", history->count, history->buffer[history->count]);
                ++history->count;
            }
            history->tmp_count = history->count;
            history->buffer[history->count][0] = '\0';
            init_cli(&CLI, history->buffer[history->count]);
            printw("\nEnter> ");
            break;

        case KEY_BACKSPACE:
            if (!empty_stack(left))
            {
                pop_stack(left);
                // printw("\b");
                move_x_delta(&cursor, -1);
                clrtoeol();
                printw("%s", right->value + right->top);
                move_x_pos(&cursor, left->top + 1);
            }
            break;

        default:
            push_stack(left, ch);
            printw("%c", ch);
            clrtoeol();
            printw("%s", right->value + right->top);
            move_x_pos(&cursor, left->top + 1);
            break;
        }
    }

    endwin();
    return 0;
}