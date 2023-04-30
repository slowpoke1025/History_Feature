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
    int background;
    char *filename;

} History;

typedef struct Args
{
    char *args[MAX_COMMAND / 2 + 1];
    int size;
    int paralle;
} Args;

History *init_history(char *filename);
void write_history();
int history_command(Args *A, History *History);

int exc_replace(Args *A, History *history);

int replace(char **ptr, char *str, char *sub, char *tar);
int _form_args(Args *A, char *str, int free);
void _free_args(Args *A);
int get_n_par(Args *A, char *par);

#endif
