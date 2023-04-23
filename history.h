#ifndef HISTORY_H
#define HISTORY_H
#define MAX_HISTORY 100
#define MAX_COMMAND 1024
#include <stdio.h>

typedef struct History
{
    char buffer[MAX_HISTORY][MAX_COMMAND];
    FILE *origin_file;
    FILE *update_file;
    int count;
    int tmp_count;
    char *filename;

} History;

History *init_history(char *filename);
void write_history();
#endif
