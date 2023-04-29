#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "cursor.h"
#include "history.h"
#include <unistd.h>
#include <sys/wait.h>

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

    // char *(*ptr)[MAX_COMMAND / 2 + 1] = &args;

    printw("Enter> ");
    refresh();

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
            history->file = fopen(history->filename, "a");
            if (!history->file)
            {
                puts("Failed to update the (*history) file.");
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

                // char *args[MAX_COMMAND / 2 + 1];

                char *args[MAX_COMMAND / 2 + 1];
                for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
                {
                    args[i] = (char *)malloc(sizeof(char) * (MAX_COMMAND / 2 + 1));
                }
                int len = form_args(args, history, history->count);
                int res;

                if ((res = exc_command(history, args, len)) == 1)
                    len = form_args(args, history, history->count);

                printw("\n%d %s", history->count + 1, history->buffer[history->count]);
                refresh();

                if (res != 0 && !history_command(history, args, len)) // res == 0 || history_command(history, args, len)
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

                        execvp(args[0], args);

                        exit(1);
                    }
                    else // parent
                    {
                        printw("\n---------\n");
                        if (history->background)
                        {
                            printw("\n[%d]", id);
                        }
                        else
                        {
                            wait(NULL);

                            char buferr[BUFFER_SIZE];
                            char buf[BUFFER_SIZE];

                            close(fd[1]);    // Close the write end of the pipe
                            close(fderr[1]); // Close the write end of the pipe

                            int n;
                            while ((n = read(fderr[0], buferr, 1)) > 0)
                            {
                                buferr[n] = '\0';
                                printw("%s", buferr);
                            }
                            close(fderr[0]);

                            while ((n = read(fd[0], buf, 1)) > 0)
                            {
                                if (buf[0] == 27)
                                {
                                    printw("Command '%s' not found\n", args[0]);
                                    close(fd[0]);
                                    break;
                                }
                                buf[n] = '\0';
                                printw("%s", buf);
                                refresh();
                            }
                            close(fd[0]);
                        }

                        printw("---------\n");
                    }
                }
                ++history->count;
                for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
                {
                    free(args[i]);
                }
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