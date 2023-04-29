#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 513

int form_args(char *(arg)[SIZE], char *str)
{
    char copy[SIZE];
    strcpy(copy, str);
    char *token;
    // char **arg = ptr;
    token = strtok(copy, " ");
    int i = 0;
    while (token)
    {
        // arg[i++] = token;
        strcpy(arg[i++], token);
        token = strtok(NULL, " ");
    }
    return i;
}
int main()
{
    char *args[SIZE];

    for (int i = 0; i < SIZE; ++i)
    {
        args[i] = (char *)malloc(sizeof(char) * 1024);
    }

    char str[] = "aa bb cc dd ee";

    int s = form_args(args, str);
    for (int i = 0; i < s; i++)
        printf("%s\n", args[i]);
    return 0;
}
