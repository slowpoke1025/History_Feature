#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "cursor.h"
#include "history.h"
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

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

void init_window()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    start_color(); // Enable color mode

    // init_color(COLOR_RED, 254 * 1000 / 255, 161 * 1000 / 255, 191 * 1000 / 255);
    // init_color(COLOR_CYAN, 0 * 1000 / 255, 231 * 1000 / 255, 255 * 1000 / 255);
    // init_color(COLOR_GREEN, 98 * 1000 / 255, 210 * 1000 / 255, 162 * 1000 / 255);
    // init_color(COLOR_MAGENTA, 164 * 1000 / 255, 89 * 1000 / 255, 209 * 1000 / 255);

    init_pair(1, COLOR_RED, COLOR_BLACK);     // Define color pair
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Define color pair
    init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Define color pair
    init_pair(4, COLOR_GREEN, COLOR_BLACK);   // Define color pair
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Define color pair

    // attrset(A_NORMAL);
    // attrset(COLOR_PAIR(1)); // Set text color to red
}

typedef struct Search_obj
{
    int i;
    char *res;
    int size;
} Search_obj;

Search_obj search(History *history, Stack *left, int base)
{

    if (base < 0)
    {
        base = history->count;
    }
    Search_obj s_obj = {base - 1, NULL, 0};

    if (!empty_stack(left))
    {
        s_obj.size = left->top + 1;
        for (; s_obj.i >= 0; --s_obj.i)
        {
            if ((s_obj.res = strstr(history->buffer[s_obj.i], left->value)))
            {
                return s_obj;
            }
        }
    }

    return s_obj;
}

void printw_search(Search_obj *s, History *history)
{
    char *ptr = history->buffer[s->i];

    printw("[");
    while (ptr != s->res)
        printw("%c", *ptr++);

    attrset(COLOR_PAIR(2)); // Set text color to red

    for (int i = 0; i < s->size; ++i)
    {
        printw("%c", *ptr++);
    }

    attrset(A_NORMAL);
    printw("%s", ptr);
    printw("]");
    refresh();
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
    refresh();

    while (1)
    {

        int ch = getch();

        switch (ch)
        {
        case KEY_UP:

            clean_line(&cursor);
            // close_stack(left);
            sprintf(history->buffer[history->tmp_count], "%s%s", left->value, right->value + right->top);

            char *prev_commond;
            if (history->tmp_count == 0)
            {
                attrset(COLOR_PAIR(1));
                printw("No prev command!\n");
                attrset(A_NORMAL);

                prev_commond = history->buffer[history->tmp_count];
            }
            else
            {
                prev_commond = history->buffer[--history->tmp_count];
            }

            init_cli(&CLI, prev_commond);

            printw("Enter> ");
            attrset(COLOR_PAIR(4));
            printw("%s", prev_commond);
            attrset(A_NORMAL);

            break;

        case KEY_CTRL('r'):

            char str[MAX_COMMAND];
            clean_line(&cursor);
            attrset(COLOR_PAIR(5));
            printw("(search)`");
            attrset(A_NORMAL);
            printw("%s", left->value);
            attrset(COLOR_PAIR(5));
            printw("': ");
            attrset(A_NORMAL);

            Search_obj s = search(history, left, -1);

            if (s.res)
            {
                // printw("[%s]", history->buffer[s.i]);
                printw_search(&s, history);
            }
            while (1)
            {
                move_x_pos(&cursor, "(search)`", left->top + 1);
                int rc = getch();
                if (isprint(rc))
                {
                    push_stack(left, rc);
                    printw("%c", rc);
                    attrset(COLOR_PAIR(5));
                    printw("': ");
                    attrset(A_NORMAL);

                    clrtoeol();
                    s = search(history, left, -1);
                    if (s.res)
                    {
                        printw_search(&s, history);
                    }
                }
                else if (rc == KEY_BACKSPACE)
                {
                    if (!empty_stack(left))
                    {
                        pop_stack(left);
                        move_x_delta(&cursor, -1);
                        clrtoeol();
                        printw("': ");

                        if (!empty_stack(left))
                        {
                            // if (!s.res)
                            {
                                s = search(history, left, -1);
                            }
                            if (s.res)
                            {
                                printw_search(&s, history);
                            }
                        }
                    }
                }
                else if (rc == '\n')
                {
                    clean_line(&cursor);
                    break;
                }
                else if (rc == KEY_CTRL('r'))
                {
                    Search_obj _s = search(history, left, s.i);
                    if (_s.res)
                    {
                        s = _s;
                        clrtoeol();
                        printw("': ");
                        printw_search(&s, history);
                    }
                }
                else if (rc == KEY_LEFT || rc == KEY_RIGHT)
                {

                    clean_line(&cursor);
                    char *command;
                    if (s.res)
                    {
                        command = history->buffer[s.i];
                        init_cli(&CLI, command);
                    }
                    else
                    {
                        command = left->value;
                    }

                    printw("Enter> ");
                    attrset(COLOR_PAIR(4));
                    printw("%s", command);
                    attrset(A_NORMAL);
                    break;
                }
                else
                {
                }
            }

            break;
        case KEY_CTRL('d'):
            history->file = fopen(history->filename, "a");
            if (!history->file)
            {
                printw("Failed to update the (*history) file.");
                refresh();
                exit(1);
            }

            for (int i = history->file_count; i < history->count; ++i)
            {

                fprintf(history->file, "%s\n", history->buffer[i]);
            }
            fclose(history->file);

            // remove(history->filename);
            // rename("_h.txt", "h.txt");
            free(history);

            exit(0);
            break;

        case KEY_DOWN:
            clean_line(&cursor);
            // close_stack(left);
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
            printw("Enter> ");
            attrset(COLOR_PAIR(4));
            printw("%s", next_command);
            attrset(A_NORMAL);
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

                Args A;
                _form_args(&A, history->buffer[history->count], FALSE);

                if (exc_command(&A, history) && !history_command(&A, history)) // res == 0 || history_command(history, args, len)
                {
                    int fd[2];
                    int fderr[2];
                    if (pipe(fd) == -1)
                    {
                        perror("pipe");
                        exit(1);
                    }
                    if (pipe(fderr) == -1)
                    {
                        perror("pipe");
                        exit(1);
                    }

                    int id = fork();
                    if (id == 0) // child
                    {
                        if (dup2(fd[1], STDOUT_FILENO) == -1)
                        {
                            perror("dup2");
                            exit(1);
                        }
                        if (dup2(fderr[1], STDERR_FILENO) == -1)
                        {
                            perror("dup2");
                            exit(1);
                        }
                        close(fd[0]);
                        close(fderr[0]);
                        close(fd[1]);
                        close(fderr[1]);
                        execvp(A.args[0], A.args);

                        exit(1);
                    }
                    else // parent
                    {

                        printw("\n---------\n");
                        refresh();

                        if (A.paralle)
                        {
                            printw("\n[%d]", id);
                            refresh();
                        }
                        else
                        {
                            wait(NULL);

                            char buferr[BUFFER_SIZE];
                            char buf[BUFFER_SIZE];

                            close(fd[1]);    // Close the write end of the pipe
                            close(fderr[1]); // Close the write end of the pipe

                            int n;
                            attrset(COLOR_PAIR(1)); // Set text color to red

                            while ((n = read(fderr[0], buferr, 1)) > 0)
                            {
                                buferr[n] = '\0';
                                printw("%s", buferr);
                            }
                            close(fderr[0]);

                            attrset(COLOR_PAIR(3)); // Set text color to red

                            while ((n = read(fd[0], buf, 1)) > 0)
                            {
                                if (buf[0] == 27)
                                {
                                    attrset(COLOR_PAIR(1)); // Set text color to red

                                    printw("Command '%s' not found\n", A.args[0]);
                                    close(fd[0]);
                                    break;
                                }
                                buf[n] = '\0';
                                printw("%s", buf);
                                refresh();
                            }
                            close(fd[0]);
                        }
                        attrset(A_NORMAL); // Set text color to red

                        printw("---------\n");
                    }
                }
                ++history->count;
                _free_args(&A);
            }
            history->tmp_count = history->count;
            history->buffer[history->count][0] = '\0';
            init_cli(&CLI, history->buffer[history->count]);
            printw("\nEnter> ");
            refresh();
            break;

        case KEY_BACKSPACE:
            if (!empty_stack(left))
            {
                pop_stack(left);
                // printw("\b");
                move_x_delta(&cursor, -1);
                clrtoeol();
                printw("%s", right->value + right->top);
                move_x_pos(&cursor, "Enter> ", left->top + 1);
            }
            break;

        default:
            push_stack(left, ch);
            attrset(COLOR_PAIR(4)); // Set text color to red
            printw("%c", ch);
            clrtoeol();
            printw("%s", right->value + right->top);
            attrset(A_NORMAL);
            refresh();
            move_x_pos(&cursor, "Enter> ", left->top + 1);
            break;
        }
    }

    endwin();
    return 0;
}