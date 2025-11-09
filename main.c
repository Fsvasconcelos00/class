#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_LINES 10  // number of lines per screen
#define MAX_LINES 10000 // maximum lines to cache positions

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *input_file;
    input_file = fopen(argv[1], "r");

    if (!input_file)
    {
        perror("Failed to open file");
        return 1;
    }

    // --- Terminal setup --- Turn off buffered input (canonical mode) and echo
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);      // save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);    // disable canonical mode & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int current_line = 0;
    int total_lines = 0;    
    char line[1024];
    long positions[MAX_LINES]; // store positions for rewind

    positions[0] = ftell(input_file); // position at start of file

    // --- Pre-scan file line positions ---
    while (fgets(line, sizeof(line), input_file) && total_lines < MAX_LINES - 1)
    {
        positions[++total_lines] = ftell(input_file);
    }

    printf("\033[2J"); // clear screen
    printf("Mini-Less Viewer — %s\n", argv[1]);
    printf("Use ↑/↓ to scroll, 'q' to quit.\n\n");

    // --- Main view loop ---
    uint8_t ch;
    bool running = true;

    while(running)
    {
        // Clear screen before redrawing
        printf("\033[H\033[J");

        printf("Mini-Less Viewer — %s\n", argv[1]);
        printf("Use ↑/↓ to scroll, 'q' to quit. Showing lines %d-%d of ~%d\n\n",
               current_line + 1,
               current_line + PAGE_LINES,
               total_lines);

        // Print a page of lines
        fseek(input_file, positions[current_line], SEEK_SET);
        for (int i = 0; i < PAGE_LINES; i++)
        {
            if (fgets(line, sizeof(line), input_file))
            {
                printf("%4d | %s", current_line + i + 1, line);
            }
            else
            {
                break;
            }
        }

        ch = getchar();
        if (ch == 'q')
        {
            running = false;
        }
        else if ((ch == 27) && (getchar() == '['))
        {               
            switch (getchar())
            {
                case 'A': // UP arrow
                    if (current_line > 0)
                    {
                        current_line--;
                    }
                    else
                    {
                        current_line = 0;
                    }
                    break;
                case 'B': // DOWN arrow
                    if (current_line + PAGE_LINES < total_lines)
                    {
                        current_line++;
                    }
                    break;
            }
        }
        else if(ch == ' ')
        {
            if ((current_line + PAGE_LINES) < total_lines)
            {
                current_line += PAGE_LINES;
            }
        }
        else if(ch == 'b')
        {
            if ((current_line - PAGE_LINES) >= 0)
            {
                current_line -= PAGE_LINES;
            }
        }
        else if(ch == 'g')
        {
            current_line = 0;
        }
        else if(ch == 'G')
        {
            current_line = total_lines - 1;
        }
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fclose(input_file);
    printf("\033[H\033[JExiting Mini-Less...\n");

    return 0;
}