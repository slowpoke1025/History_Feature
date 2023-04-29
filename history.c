#include <stdlib.h>
#include <string.h>
#include "history.h"
#include <ncurses.h>
#include <unistd.h>

History *__history;

History *init_history(char *filename)
{
    History *history = (History *)malloc(sizeof(History));
    if (filename == NULL)
        filename = "h.txt";
    history->filename = filename;

    history->file = fopen(filename, "r");

    if (!history->file)
    {
        history->file = fopen(filename, "w");
        if (!history->file)
        {
            puts("Failed to create the history file.");
            exit(1);
        }
    }
    char entry[MAX_COMMAND];
    history->count = 0;
    history->base_count = 0;
    while (fgets(entry, MAX_COMMAND, history->file))
    {
        entry[strcspn(entry, "\n")] = '\0';
        strcpy(history->buffer[history->count++], entry);
    }
    history->tmp_count = history->file_count = history->count;
    fclose(history->file);
    atexit(write_history);
    __history = history;
    return history;
}

void write_history()
{
    endwin();
}

int history_command(Args *A, History *history)
{
    if (strcmp(A->args[0], "history") == 0)
    {
        if (A->size == 1)
        { // 全部history
            for (int i = 0; i <= history->count; ++i)
            {
                printw("\n%4d %s", i + 1, history->buffer[i]);
                refresh();
            }
        }
        else if (A->size == 2 && A->args[1][0] == '-' && strlen(A->args[1]) == 2)
        {
            switch (A->args[1][1])
            {
            case 'w':
                break;
            case 'c':
                break;
            default:
                printw("\nhistory invalid option: -- \'%s\'", A->args[1]);
                break;
            }
        }
        return 1;
    }
    return 0;
}

int exc_command(Args *A, History *history)
{
    char *ptr = history->buffer[history->count];
    if (!strchr(ptr, '!'))
    {
        return -1;
    }
    if (history->count - history->base_count == 0)
    {
        printw("\nNo command in history.");
        return 0;
    }

    if (A->args[0][0] == '!')
    {
        int n;

        if (A->args[0][1] == '!')
        {
            printw("\n[!! -> %s]", history->buffer[history->count - 1]);
            strcpy(history->buffer[history->count], history->buffer[history->count - 1]);
            _form_args(A, history->buffer[history->count], FALSE);
            return 1;
        }
        else if ((n = atoi(A->args[0] + 1)))
        {
            int index;
            if (n > history->base_count && n <= history->count)
            {
                index = n - 1;
            }
            else if (n < 0 && -n <= history->count - history->base_count)
            {
                index = history->count + n;
            }
            else
            {
                printw("\nNo such command in history");
                return 0;
            }

            printw("\n[!%d -> %s]", n, history->buffer[index]);
            strcpy(history->buffer[history->count], history->buffer[index]);
            _form_args(A, history->buffer[history->count], FALSE);
            return 1;
        }
        else
        {
            for (int i = history->count - 1; i >= 0; --i)
            {
                if (strncmp(A->args[0] + 1, history->buffer[i], strlen(A->args[0] + 1)) == 0)
                {
                    printw("\n[!%d -> %s (%s)]", i + 1, history->buffer[i], A->args[0] + 1);
                    strcpy(history->buffer[history->count], history->buffer[i]);

                    _form_args(A, history->buffer[history->count], FALSE);

                    return 1;
                }
            }
            printw("\n! event not found: -- \'%s\'", A->args[0] + 1);
            return 0;
        }
    }
    else
    {
        _form_args(A, history->buffer[history->count - 1], FALSE);

        int flag = 0, _flag = 0;
        char *ptr = history->buffer[history->count];

        if (A->size > 1)
        {
            flag += replace(&ptr, ptr, "!$", A->args[A->size - 1]);
            flag += replace(&ptr, ptr, "!^", A->args[1]);
        }

        while (1)
        {
            int exc = strcspn(ptr, "!");
            int col = strcspn(ptr, ":");
            int del = col - exc + 1;
            int size = strlen(ptr);
            if (exc == size || col == size)
            {
                break;
            }

            if (del <= 2)
            {
                flag = 0;
                break;
            }

            char str[MAX_COMMAND];
            int pn_len = strcspn(ptr + exc + del, " ");

            strncpy(str, ptr + exc, del + pn_len);

            str[del + pn_len] = '\0';

            printw("\nexc=[%d], col=[%d], del=[%d]", exc, col, del);
            printw("\nstr=[%s], pn_len=[%d]", str, pn_len);

            refresh();

            int n, n_par;
            char *par = ptr + col + 1;

            if ((n = atoi(ptr + exc + 1)))
            {
                int index;
                if (n > history->base_count && n <= history->count)
                {
                    index = n - 1;
                }
                else if (n < 0 && -n <= history->count - history->base_count)
                {
                    index = history->count + n;
                }
                else
                {
                    // printw("\nNo such command in history");
                    flag = 0;
                    break;
                }

                _form_args(A, history->buffer[index], FALSE);

                n_par = get_n_par(A, par);
                if (!n_par)
                {
                    flag = 0;
                    break;
                }
                flag += replace(&ptr, ptr, str, A->args[n_par]);
            }
            else
            {
                int _flag = flag;
                char cmd[MAX_COMMAND];
                int len_cmd = del - 2;
                strncpy(cmd, str + 1, len_cmd);
                cmd[len_cmd] = '\0';

                for (int i = history->count - 1; i >= 0; --i)
                {
                    if (strncmp(cmd, history->buffer[i], len_cmd) == 0)
                    {
                        _form_args(A, history->buffer[i], FALSE);
                        n_par = get_n_par(A, par);
                        if (n_par)
                        {
                            flag += replace(&ptr, ptr, str, A->args[n_par]);
                            printw("\nn_par=%s, ptr=%s", A->args[n_par], ptr);
                            refresh();
                            break;
                        }
                        flag = 0;
                        break;
                    }
                }

                if (flag == _flag)
                {
                    printw("\n! event not found: -- \'%s\'", str);
                    return 0;
                }
            }
        }

        _form_args(A, history->buffer[history->count], TRUE);

        if (!flag)
        {
            return -1;
        }
        return 1;
    }
}

int get_n_par(Args *A, char *par)
{
    int n_par;
    if (*par == '$')
    {
        n_par = A->size - 1;
    }
    else if (*par == '^')
    {
        n_par = 1;
    }
    else
    {
        n_par = atoi(par);
    }

    if (!n_par || A->size <= n_par)
        return 0;
    return n_par;
}

int replace(char **ptr, char *str, char *sub, char *tar)
{
    printw("\nreplace %s -> %s", sub, tar);
    int sub_len = strlen(sub);
    int tar_len = strlen(tar);
    char res[MAX_COMMAND];
    char *p = res;
    int i = 0;

    while (*str)
    {
        if (strncmp(str, sub, sub_len) == 0)
        {
            strncpy(p, tar, tar_len);
            p += tar_len;
            str += sub_len;
            ++i;
        }
        else
        {
            *p++ = *str++;
        }
    }
    *p = '\0';
    strcpy(*ptr, res);

    return i;
}

void _free_args(Args *A)
{
    for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
    {
        free(A->args[i]);
    }
}

int _form_args(Args *A, char *str, int free)
{
    if (free)
        _free_args(A);

    char copy[MAX_COMMAND];
    strcpy(copy, str);
    // if (!free)
    for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
    {
        A->args[i] = malloc(sizeof(char) * MAX_COMMAND);
    }

    refresh();
    char *token = strtok(copy, " ");
    int i = 0, len = 0;
    while (token != NULL)
    {
        printw("\n[%s]", token);
        refresh();
        strcpy(A->args[i], token);
        printw("{%s}", A->args[i]);
        refresh();
        ++i;
        token = strtok(NULL, " ");
    }

    if (A->args[i - 1][0] == '&')
    {
        A->paralle = 1;
        A->args[i - 1] = NULL;
        A->size = i - 1;
    }
    else
    {
        A->paralle = 0;
        A->args[i] = NULL;
        A->size = i;
    }
}

int form_args(char *(args)[MAX_COMMAND / 2 + 1], History *history, int count)
{
    char cmd[MAX_COMMAND];
    strcpy(cmd, history->buffer[count]);
    char *token = strtok(cmd, " ");
    int i = 0, len;
    while (token != NULL)
    {
        printw("\n[%s]", token);
        refresh();
        strcpy(args[i], token);
        printw("\n{%s}", args[i]);
        refresh();
        ++i;
        token = strtok(NULL, " ");
    }

    if (args[i - 1][0] == '&')
    {
        history->background = 1;
        printw("[background]\n");
        args[i - 1] = NULL;

        len = i - 1;
    }
    else
    {
        history->background = 0;
        args[i] = NULL;
        len = i;
    }
    printw("\n--end form--");
    refresh();
    return len;
}
