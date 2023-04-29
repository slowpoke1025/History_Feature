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

int history_command(History *history, char *args[MAX_COMMAND / 2 + 1], int len)
{
    if (strcmp(args[0], "history") == 0)
    {
        if (len == 1)
        { // 全部history
            for (int i = 0; i <= history->count; ++i)
            {
                printw("\n%4d %s", i + 1, history->buffer[i]);
                refresh();
            }
        }
        else if (len == 2 && args[1][0] == '-' && strlen(args[1]) == 2)
        {
            switch (args[1][1])
            {
            case 'w':
                break;
            case 'c':
                break;
            default:
                printw("\nhistory invalid option: -- \'%s\'", args[1] + 1);
                break;
            }
        }

        return 1;
    }
    return 0;
}

int exc_command(History *history, char *args[MAX_COMMAND / 2 + 1], int len)
{
    if (!strchr(history->buffer[history->count], '!'))
    {
        return -1;
    }
    if (args[0][0] == '!')
    {

        if (history->count - history->base_count == 0)
        {
            printw("\nNo command in history.");
            return 0;
        }

        int n;

        if (args[0][1] == '!' && strlen(args[0]) == 2)
        {
            printw("\n[!! -> %s]", history->buffer[history->count - 1]);
            strcpy(history->buffer[history->count], history->buffer[history->count - 1]);
            return 1;
        }
        else if ((n = atoi(args[0] + 1)))
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
            return 1;
        }
        else
        {
            for (int i = history->count - 1; i > 0; --i)
            {
                if (strstr(history->buffer[i], args[0] + 1) == history->buffer[i])
                {
                    printw("\n[!%d -> %s]", i + 1, history->buffer[i]);
                    strcpy(history->buffer[history->count], history->buffer[i]);
                    return 1;
                }
            }
            printw("\n! event not found: -- \'%s\'", args[0] + 1);
            return 0;
        }
    }
    else
    {
        char *_args[MAX_COMMAND / 2 + 1];
        for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
        {
            _args[i] = (char *)malloc(sizeof(char) * (MAX_COMMAND / 2 + 1));
        }

        int _len = form_args(_args, history, history->count - 1);
        int flag = 0;
        if (_len > 1)
        {
            char *ptr = history->buffer[history->count];
            flag += replace(&ptr, ptr, "!$", _args[_len - 1]);
            flag += replace(&ptr, ptr, "!^", _args[1]);
        }
        for (int i = 0; i < MAX_COMMAND / 2 + 1; ++i)
        {
            free(_args[i]);
        }
        // form_args(args, history, history->count); if flag

        if (!flag)
        {
            return -1;
        }
        return 1;
    }
}
int replace(char **ptr, char *str, char *sub, char *tar)
{
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
