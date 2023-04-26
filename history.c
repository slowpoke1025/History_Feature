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
    if (args[0][0] == '!' && len == 1)
    {
        int n;

        if (args[0][1] == '!' && strlen(args[0]) == 2)
        {
            if (history->count - history->base_count > 0)
            {
                printw("\n[!! -> %s]", history->buffer[history->count - 1]);
                strcpy(history->buffer[history->count], history->buffer[history->count - 1]);
                return 1;
            }
            else
            {
                printw("\nNo command in history.");
                return 0;
            }
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
            printw("\n! invalid option: -- \'%s\'", args[0] + 1);
            return 0;
        }
    }
    return -1;
}
