#define USERNAME_COLOR 1
#define COMMAND_COLOR 2
#define OUTPUT_COLOR 3
#define ERROR_COLOR 4
#include <ncurses.h>
int main()
{
    FILE *fp = fopen("h.txt", "w");
    for (size_t i = 1; i <= 10; i++)
    {
        fprintf(fp, "--%d--\n", i);
    }
    fclose(fp);
    int i = 5;
    printf("%d", --i % 3);
}
