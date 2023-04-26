#ifndef HISTORY_H
#define HISTORY_H
#define MAX_HISTORY 100
#define MAX_HISTFILE 200

#define MAX_COMMAND 1024
#include <stdio.h>

typedef struct History
{
    char buffer[MAX_HISTORY][MAX_COMMAND];
    FILE *file;
    int count;
    int tmp_count;
    int file_count;
    int base_count;

    char *filename;

} History;

History *init_history(char *filename);
void write_history();
int history_command(History *history, char *args[MAX_COMMAND / 2 + 1], int len);
int exc_command(History *history, char *args[MAX_COMMAND / 2 + 1], int len);

#endif
