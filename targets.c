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
#include <math.h>

struct data data;
int targetReached = 0;


FILE* file;
sem_t* LOGsem; 
char msg[100];


void logit(char *msg){
      sem_wait(LOGsem);

        file = fopen( LOGPATH, "a");
        // Check if the file was opened successfully
        if (file == NULL) {
            fprintf(stderr, "Error opening the file.\n");
            exit(EXIT_FAILURE);
        }

        // Write the string to the file
        fprintf(file, "%s\n", msg);
        // Close the file
        fclose(file);
        sem_post(LOGsem);
}

int main(int argc, char *argv[])
{
      // Create or open a semaphore for logging
    LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); 
    if (LOGsem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

     // PIPES
    int  targets_server[2], server_targets[2];
    sscanf(argv[0], "%d %d|%d %d",  &targets_server[0], &targets_server[1], &server_targets[0], &server_targets[1]);
    close(targets_server[0]); //Close unnecessary pipes
    close(server_targets[1]);

    sprintf(msg,"[Targets]: ALIVE");
    logit(msg);

        
    read(server_targets[0], &data, sizeof(data));

    //create targets
    for (int i = 0; i < NUM_TARGETS * 2; i += 2) {
        do {
            data.targets[i] = rand() % (int)data.max[0];
        } while (fabs(data.targets[i] - data.drone_pos[0]) < THRESH_TARGET);

        do {
            data.targets[i + 1] =rand() % (int)data.max[1];
        } while (fabs(data.targets[i + 1] - data.drone_pos[1]) < THRESH_TARGET);

        sprintf(msg, "[Targets]: x[%d]: %f, y[%d]: %f", i/2, data.targets[i], i/2, data.targets[i + 1]);
        logit(msg);
    }
    write(targets_server[1], &data, sizeof(data));

   while (1) {
    read(server_targets[0], &data, sizeof(data));
    // Check if drone reached any targets
    for (int i = targetReached; i < NUM_TARGETS; i++) {
        if (data.targets[i * 2] != -1) {
            targetReached = i;
            break;
        }
    }
            // Check the distance between the drone and the target
            double distance = sqrt(pow(data.drone_pos[0] - data.targets[targetReached * 2], 2) +
                                   pow(data.drone_pos[1] - data.targets[targetReached * 2 + 1], 2));

            if (distance < THRESH_TOUCH) {
                // Drone has reached the target, update the target status
                data.targets[targetReached * 2] = -1;
                data.targets[targetReached * 2 + 1] = -1;

                data.targetReached = targetReached;
                // Write the updated data to the targets_server pipe
                write(targets_server[1], &data, sizeof(data));
                // Log the information
                sprintf(msg, "[Targets]: Drone reached target %d at (%f, %f)", targetReached, data.targets[targetReached * 2], data.targets[targetReached * 2 + 1]);
                logit(msg);
            }
        }

    //cleanup
    close(targets_server[1]); 
    close(server_targets[0]);

}