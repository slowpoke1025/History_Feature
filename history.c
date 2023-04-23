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

    history->origin_file = fopen(filename, "r");

    if (!history->origin_file)
    {
        history->origin_file = fopen(filename, "w");
        if (!history->origin_file)
        {
            puts("Failed to create the history file.");
            exit(1);
        }
    }
    char entry[MAX_COMMAND];
    history->count = 0;
    while (fgets(entry, MAX_COMMAND, history->origin_file))
    {
        entry[strcspn(entry, "\n")] = '\0';
        strcpy(history->buffer[history->count++], entry);
    }
    history->tmp_count = history->count;
    fclose(history->origin_file);
    atexit(write_history);
    __history = history;
    return history;
}

void write_history()
{
    __history->update_file = fopen("_h.txt", "w");
    if (!__history->origin_file)
    {
        puts("Failed to update the (*__history) file.");
        exit(1);
    }

    for (int i = 0; i < __history->count; ++i)
    {
        fprintf(__history->update_file, "%s\n", __history->buffer[i]);
    }
    fclose(__history->update_file);

    remove(__history->filename);
    rename("_h.txt", "h.txt");
    free(__history);
    endwin();
}