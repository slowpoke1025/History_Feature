#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 513

int form_args(char *(arg)[SIZE], char *str)
{
    for (int i = 0; i < SIZE; ++i)
    {
        strcpy(arg[i], "hello world");
    }
}
int main()
{
    char str[] = "ssd";
    printf("%d", strcpy(str, "hd\0d")[2] == '\0');
    return 0;
}
