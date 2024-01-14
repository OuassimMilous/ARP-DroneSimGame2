#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <time.h>
#include "include/constants.h"

int main(int argc, char *argv[])
{
    // Initialize ncurses
    initscr();


    //declaration of variables
    char keyPressed;
    struct data data;
    char myChar;
    // // Open the semaphore for FIFO
    // sem_t* semFIFO = sem_open(SEMFIFOPATH, O_RDWR, 0);
    // if (semFIFO == SEM_FAILED) {
    //     perror("sem_open3");
    //     exit(EXIT_FAILURE);
    // }
    // sem_post(semFIFO);

    // Semaphore for logging
    sem_t *LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); // Initial value is 1
    if (LOGsem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_post(LOGsem);


    int keyboard_server[2], server_keyboard[2];
    sscanf(argv[1], "%d %d|%d %d",  &keyboard_server[0], &keyboard_server[1], &server_keyboard[0], &server_keyboard[1]);
    close(keyboard_server[0]); 
    close(server_keyboard[1]);

 
    // Set non-blocking input
nodelay(stdscr, TRUE);
    
    while (1)
    {

        keyPressed = getch();

        if (keyPressed != ERR)
        {
            data.key=keyPressed;
        }
        else
        {
            data.key=myChar;
        }
           write(keyboard_server[1],&data,sizeof(data));
 
            sem_wait(LOGsem);

            // Open the file in append mode
            FILE *file = fopen(LOGPATH, "a");
            if (file == NULL)
            {
                fprintf(stderr, "Error opening the file.\n");
                exit(EXIT_FAILURE);
            }

            // Write the log message
            fprintf(file, "[Keyboard]: key %c pressed\n", keyPressed);

            // Close the file
            fclose(file);

            sem_post(LOGsem);

        // Sleep for 20 milliseconds
        usleep(50000);
    }

       // Clean up
    close(keyboard_server[1]);
    close(keyboard_server[0]); 


    // End ncurses
    endwin();


    return 0;
}
