#include <stdlib.h>
#include <string.h>
#include "history.h"
#include <ncurses.h>
#include <unistd.h>

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
    while (fgets(entry, MAX_COMMAND, history->file))
    {
        entry[strcspn(entry, "\n")] = '\0';
        strcpy(history->buffer[history->count], entry);
        if (++history->count == MAX_HISTORY - 1)
        {
            break;
        }
    }

    history->tmp_count = history->count;
    for (int i = history->count; i < MAX_HISTORY; ++i)
    {
        history->buffer[i][0] = '\0';
    }
    fclose(history->file);
    // atexit(write_history);
    // __history = history;
    return history;
}
int isFull(History *history)
{
    return history->count >= MAX_HISTORY - 1;
}
int get_oldest(History *history)
{
    return history->count < MAX_HISTORY ? 0 : (history->count + 1);
}
int get_count(History *history)
{
    return !isFull(history) ? history->count : MAX_HISTORY - 1;
}

void write_history(History *history)
{

    history->file = fopen("_h.txt", "w");
    if (!history->file)
    {
        printw("Failed to update the (*history) file.");
        refresh();
        exit(1);
    }

    int count = get_count(history);
    int base = get_oldest(history);

    for (int i = 0; i < count; ++i)
    {
        fprintf(history->file, "%s\n", history->buffer[(base + i) % MAX_HISTORY]);
    }
    fclose(history->file);

    remove(history->filename);
    rename("_h.txt", history->filename);
}

int history_command(Args *A, History *history)
{
    if (strcmp(A->args[0], "history") == 0)
    {
        int count = get_count(history);
        if (count == 0)
        {
            attrset(COLOR_PAIR(1));
            printw("\nNo command in history.");
            attrset(A_NORMAL);
            return 1;
        }
        int n;

        if (A->size == 1)
        {
            int base = get_oldest(history);
            int pos = history->count - count;

            if (isFull(history))
            {
                ++base;
                ++pos;
                --count;
            }

            attrset(COLOR_PAIR(3));
            for (int i = 1; i <= count + 1; ++i)
            {
                printw("\n%4d %s", pos + i, history->buffer[base % MAX_HISTORY]);
                ++base;
                refresh();
            }
            attrset(A_NORMAL);
        }

        else if ((n = atoi(A->args[1])))
        {
            n = n > count ? count : n;

            for (int i = n; i > 0; --i)
            {
                int pos = history->count - i;
                printw("\n%4d %s", pos + 1, history->buffer[pos % MAX_HISTORY]);
                refresh();
            }
        }

        else if (A->args[1][0] == '-' && strlen(A->args[1]) == 2)
        {
            switch (A->args[1][1])
            {
            case 'w':
                printw("\nwww");
                write_history(history);
                break;
            case 'c':
                printw("\nccc");
                for (int i = 0; i < MAX_HISTORY; ++i)
                {
                    history->buffer[i][0] = '\0';
                }
                history->count = -1;
                break;
            default:
                attrset(COLOR_PAIR(1));
                printw("\nhistory invalid option: -- \'%s\'\n", A->args[1]);
                attrset(A_NORMAL);
                break;
            }
        }
        return 1;
    }
    return 0;
}

int exc_replace(Args *A, History *history)
{
    char *ptr = history->buffer[history->count % MAX_HISTORY];
    char *pptr = history->buffer[(history->count - 1) % MAX_HISTORY];
    if (!strchr(ptr, '!'))
    {
        return -1;
    }

    int count = get_count(history);
    if (count == 0)
    {
        attrset(COLOR_PAIR(1));
        printw("\nNo command in history.");
        attrset(A_NORMAL);
        return 0;
    }

    if (A->args[0][0] == '!')
    {
        int n;

        if (A->args[0][1] == '!')
        {
            printw("\n[!! -> %s]", pptr);
            strcpy(ptr, pptr);
            _form_args(A, ptr, FALSE);
            return 1;
        }
        else if ((n = atoi(A->args[0] + 1)))
        {
            int index;
            if (n > history->count - count && n <= history->count)
            {
                index = n - 1;
            }
            else if (n < 0 && -n <= count)
            {
                index = history->count + n;
            }
            else
            {
                attrset(COLOR_PAIR(1));
                printw("\nNo such command in history\n");
                attrset(A_NORMAL);

                return 0;
            }

            printw("\n[!%d -> %s]", n, history->buffer[index % MAX_HISTORY]);
            strcpy(ptr, history->buffer[index % MAX_HISTORY]);
            _form_args(A, ptr, FALSE);
            return 1;
        }
        else
        {
            int base = history->count - 1;

            for (int i = 0; i < count; ++i)
            {
                int pos = (base - i) % MAX_HISTORY;
                if (strncmp(A->args[0] + 1, history->buffer[pos], strlen(A->args[0] + 1)) == 0)
                {
                    printw("\n[!%s -> %s ]", A->args[0] + 1, history->buffer[pos]);
                    strcpy(ptr, history->buffer[pos]);
                    _form_args(A, ptr, FALSE);
                    return 1;
                }
            }

            attrset(COLOR_PAIR(1));

            printw("\n! event not found: -- \'%s\'", A->args[0] + 1);
            attrset(A_NORMAL);

            return 0;
        }
    }
    else
    {
        _form_args(A, pptr, FALSE);

        int flag = 0, _flag = 0;

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
                // if (n > get_oldest(history) && n <= history->count)
                if (n > history->count - count && n <= history->count)
                {
                    index = n - 1;
                }
                else if (n < 0 && -n <= count)
                {
                    index = history->count + n;
                }
                else
                {
                    flag = 0;
                    break;
                }

                _form_args(A, history->buffer[index % MAX_HISTORY], FALSE);

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

                int base = history->count - 1;
                for (int i = 0; i < count; ++i)
                {
                    int pos = (base - i) % MAX_HISTORY;
                    if (strncmp(cmd, history->buffer[pos], len_cmd) == 0)
                    {
                        _form_args(A, history->buffer[pos], FALSE);
                        n_par = get_n_par(A, par);
                        if (n_par)
                        {
                            flag += replace(&ptr, ptr, str, A->args[n_par]);
                            break;
                        }
                        flag = 0;
                        break;
                    }
                }

                if (flag == _flag)
                {
                    attrset(COLOR_PAIR(1));
                    printw("\n! event not found: -- \'%s\'", str);
                    attrset(A_NORMAL);

                    return 0;
                }
            }
        }

        _form_args(A, history->buffer[history->count % MAX_HISTORY], TRUE); // 迴圈終止

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
    refresh();
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
