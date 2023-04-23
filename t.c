#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MAX_HISTORY_SIZE 1000

// Global variable to hold the history buffer
char history[MAX_HISTORY_SIZE][100];
int history_count = 0;

// Function to write history buffer to file
void write_history_to_file()
{
    // Open history file for writing
    FILE *fp = fopen("history.txt", "w");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    // Write each history command to file
    for (int i = 0; i < history_count; i++)
    {
        fprintf(fp, "%s\n", history[i]);
    }

    // Close the file
    fclose(fp);
}

int main()
{
    // Register the function to write history to file on exit
    atexit(write_history_to_file);

    // Loop to read user input and update history buffer
    while (true)
    {
        printf("> ");
        fflush(stdout);

        // Read user input
        char input[100];
        if (fgets(input, 100, stdin) == NULL)
        {
            break; // Exit if user presses Ctrl-D
        }

        // Add input to history buffer
        if (history_count < MAX_HISTORY_SIZE)
        {
            strcpy(history[history_count], input);
            history_count++;
        }
        else
        {
            // Shift history buffer to make room for new input
            for (int i = 1; i < MAX_HISTORY_SIZE; i++)
            {
                strcpy(history[i - 1], history[i]);
            }
            strcpy(history[MAX_HISTORY_SIZE - 1], input);
        }
    }

    return 0;
}
