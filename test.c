
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

    printw("Enter> ");
    int pipefd[2];

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

                char cmd[MAX_COMMAND];
                char *args[MAX_COMMAND / 2 + 1];
                strcpy(cmd, history->buffer[history->count]);
                // cmd[strcspn(cmd, "\n")] = '\0';

                char *token = strtok(cmd, " ");
                int background = 0;
                int i = 0;
                while (token != NULL)
                {
                    args[i++] = token;
                    token = strtok(NULL, " ");
                }
                if (args[i - 1][0] == '&')
                {
                    background = 1;
                    printw("background");
                    args[i - 1] = NULL;
                }
                else
                {
                    args[i] = NULL;
                }

                // if (pipe(pipefd) == -1)
                // {
                //     perror("pipe");
                //     exit(1);
                // }

                int id = fork();
                if (id == 0)
                {
                    // close(pipefd[0]);

                    // if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                    // {
                    //     perror("dup2");
                    //     exit(1);
                    // }
                    // if (dup2(pipefd[1], STDERR_FILENO) == -1)
                    // {
                    //     perror("dup2");
                    //     exit(1);
                    // }
                    freopen("output.txt", "w", stdout);
                    freopen("error.txt", "w", stderr);

                    execvp(args[0], args);
                    exit(1);
                }
                wait(NULL);
                // close(pipefd[1]); // Close the write end of the pipe
                char buf[BUFFER_SIZE];

                // int n;
                printw("---------\n");

                // while ((n = read(pipefd[0], buf, BUFFER_SIZE)) > 0)
                // {
                //     buf[n] = '\0';
                //     printw("%s", buf);
                // }
                // close(pipefd[0]);

                FILE *fp = fopen("output.txt", "r");
                FILE *ferr = fopen("error.txt", "r");

                while (fgets(buf, sizeof(buf), ferr) != NULL)
                {
                    printw("%s", buf);
                }
                while (fgets(buf, sizeof(buf), fp) != NULL)
                {
                    if (buf[0] == 27)
                    {
                        printw("Command '%s' not found\n", args[0]);
                        break;
                    }
                    printw("%s", buf);
                }

                fclose(fp);
                fclose(ferr);
                printw("---------");

                // refresh();
                // close(pipefd[0]); // Close the read end of the pipe again (in case read failed)

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