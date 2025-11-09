#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

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

    int ch;
    long positions[1000]; // store positions for rewind
    int current_line = 0;

    char line[1024];
    positions[0] = ftell(input_file); // position at start of file
    while(true)
    {
        printf("\033[2J"); // clear screen
        printf("Mini-Less Viewer — %s\n", argv[1]);
        printf("Use ↑/↓ to scroll, 'q' to quit.\n\n");
        printf("%s", line);

        ch = getchar();
        if (ch == 'q') break; // quit on q
        
        if ((ch == 27) && (getchar() == '['))
        {               
            switch (getchar())
            {
                case 'A': // UP arrow
                    if (current_line > 0)
                    {
                        current_line--;
                        fseek(input_file, positions[current_line], SEEK_SET);
                        if (fgets(line, sizeof(line), input_file))
                            printf("%s", line);
                    }
                    fflush(stdout);
                    break;
                case 'B': // DOWN arrow
                    positions[current_line] = ftell(input_file);
                    if (fgets(line, sizeof(line), input_file))
                    {
                        printf("%s", line);
                        current_line++;
                    }
                    fflush(stdout);
                    break;
            }
        }
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("Exiting...\n");

    return 0;
}