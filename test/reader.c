#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

int main(int argc, char *argv[]) {

    printf("Number of command-line arguments: %d\n", argc);

    printf("Command-line arguments:\n");
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    char letter;

    // PIPES
    int reader_writer[2];
    int writer_reader[2];

    sscanf(argv[1], "%d %d|%d %d", &reader_writer[0], &reader_writer[1], &writer_reader[0], &writer_reader[1]);
    //close(reader_writer[0]);
    close(writer_reader[1]);
 

    while (1) {
                printf("waiting\n");
                    fflush(stdout);

        ssize_t bytes_read = read(writer_reader[0], &letter, sizeof(letter));
        printf("asdasdas %c \n", letter);
        fflush(stdout);


    }

    close(reader_writer[1]);
    close(writer_reader[0]);

    return 0;
}
