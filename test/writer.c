#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <ncurses.h>
#include <time.h>

int main(int argc, char *argv[]) {
    initscr();

    printf("Number of command-line arguments: %d\n", argc);

    printf("Command-line arguments:\n");
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    // PIPES
    int reader_writer[2];
    int writer_reader[2];

    sscanf(argv[1], "%d %d|%d %d", &reader_writer[0], &reader_writer[1], &writer_reader[0], &writer_reader[1]);
   // close(reader_writer[0]);
    close(writer_reader[0]);

    nodelay(stdscr, TRUE);
    char letter;


    while (1) {

        letter = getch();

        if (letter != ERR)
        {
        write(writer_reader[1], &letter, sizeof(letter));
        printf("sent\n");
        fflush(stdout);
        }
        usleep(20000);
    }

    // These statements were moved outside the loop
    close(reader_writer[1]);
    close(writer_reader[1]);
    endwin();

    return 0;
}



