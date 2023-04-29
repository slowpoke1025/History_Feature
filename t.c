#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_COMMAND 100

int replace(char **ptr, char *str, char *sub, char *tar)
{
    int sub_len = strlen(sub);
    int tar_len = strlen(tar);
    char res[MAX_COMMAND];

    char *p = res;

    while (*str)
    {
        if (strncmp(str, sub, sub_len) == 0)
        {
            strncpy(p, tar, tar_len);
            p += tar_len;
            str += sub_len;
        }
        else
        {
            *p++ = *str++;
        }
    }
    *p = '\0';
    strcpy(*ptr, res);
    return 0;
}

int main()
{
    int row = 3, col = 4, i, j, count;

    int(*arr)[row][col];

    // count = 0;
    // for (i = 0; i < row; i++)
    //     for (j = 0; j < col; j++)
    //         (*arr)[i][j] = ++count;

    // for (i = 0; i < row; i++)
    //     for (j = 0; j < col; j++)
    //         printf("%d ", (*arr)[i][j]);

    printf("%d", sizeof(arr));

    // free(arr);

    return 0;
}
