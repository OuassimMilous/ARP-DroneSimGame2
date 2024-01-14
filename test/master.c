#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <unistd.h> 
#include <stdlib.h>
#include "include/constants.h"
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    //declaration of variables
    pid_t reader, writer;
    int res;
    int num_children = 0;

    // PIPES
    int reader_writer[2];
    int writer_reader[2];

    if (pipe(reader_writer) == -1 || pipe(writer_reader) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    reader = fork();
    if (reader < 0) {
        perror("Fork error at reader");
        return -1;
    } else if (reader == 0) {
        char pipeargs[100]; 

        sprintf(pipeargs, "%d %d|%d %d",
                reader_writer[0], reader_writer[1], writer_reader[0], writer_reader[1]);

        char *arg_list[] = {"konsole", "-e", "./build/reader", pipeargs, NULL};
        execvp("konsole", arg_list);
        return 0;  // Should not reach here
    }
    num_children += 1;

    writer = fork();
    if (writer < 0) {
        perror("Fork error at writer");
        return -1;
    } else if (writer == 0) {
        char pipeargs[100]; 

        sprintf(pipeargs, "%d %d|%d %d",
                reader_writer[0], reader_writer[1], writer_reader[0], writer_reader[1]);

        char *arg_list[] = {"konsole", "-e", "./build/writer", pipeargs, NULL};
        execvp("konsole", arg_list);
        return 0;  // Should not reach here
    }
    num_children += 1;

    // Wait for all remaining children to terminate
    while (num_children > 0) {
        wait(&res);
        num_children -= 1;
    }

    // Exit the main process
    return 0;
}
