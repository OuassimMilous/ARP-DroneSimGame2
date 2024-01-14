#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <unistd.h> 
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "include/constants.h"


sem_t *LOGsem;
FILE *file;
int centered =0;

// the function to log things
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

int main(int argc, char *argv[]) {
    char msg [100];  //the variable for creating logging msgs


    // open the semaphore for logging
    LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); // Initial value is 1
    if (LOGsem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_post(LOGsem);

   // declaration of variables
    double obst_pos[NUM_OBSTACLES*2];
    double targets_pos[NUM_TARGETS*2];
    struct data data, updated_data;

    int server_UI[2];
    int server_keyboard[2];
    int server_drone[2];
    int server_obstacles[2];
    int server_targets[2];

    int rec_pipes[NUM_PROCESSES-2][2];
   //Rec_pipes in order of
    /*
    WINDOW
    KEYBOARD
    DRONE
    OBSTACLE
    TARGET
    */

    sscanf(argv[0],"%d %d|%d %d|%d %d|%d %d|%d %d|%d %d|%d %d|%d %d|%d %d|%d %d",
                                    &rec_pipes[0][0],   &rec_pipes[0][1], &server_UI[0],       &server_UI[1],
                                    &rec_pipes[1][0],   &rec_pipes[1][1], &server_keyboard[0], &server_keyboard[1],
                                    &rec_pipes[2][0],   &rec_pipes[2][1], &server_drone[0],    &server_drone[1],
                                    &rec_pipes[3][0],   &rec_pipes[3][1], &server_obstacles[0], &server_obstacles[1],
                                    &rec_pipes[4][0],   &rec_pipes[4][1], &server_targets[0],   &server_targets[1]); // Get the fds of the pipe to watchdog
    
    // Close unnecessary pipes
    close(server_drone[0]); 
    close(server_keyboard[0]);
    close(server_UI[0]);
    close(server_obstacles[0]);
    close(server_targets[0]);

    for(int i=0; i< NUM_PROCESSES-1; i++){
        close(rec_pipes[i][1]);
    }

        while(1){


    fd_set reading;
    FD_ZERO(&reading);  // Reset the set before each call to select

    for (int i = 0; i < 6; i++) {
        FD_SET(rec_pipes[i][0], &reading);
    }

    int max_pipe_fd = -1;  // Reset max_pipe_fd

    for (int i = 0; i < 6; i++) {
        if (rec_pipes[i][0] > max_pipe_fd) {
            max_pipe_fd = rec_pipes[i][0];
        }
    }

    // selecting which pipe is recieving data
    int ret_val = select(max_pipe_fd + 1, &reading, NULL, NULL, NULL);

    if (ret_val == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    }

        for(int j=0; j<(NUM_PROCESSES-2); j++){
            if(ret_val>0){

                if(FD_ISSET(rec_pipes[j][0],&reading)>0){ // Only from pipes that are updated
                    read(rec_pipes[j][0],&updated_data, sizeof(data)); 
                    switch (j){
                    case 0: //UI
                        memcpy(data.max, updated_data.max, sizeof(updated_data.max));
                        if (!centered){
                        memcpy(data.drone_pos, updated_data.drone_pos, sizeof(updated_data.drone_pos));
                        data.Cobs_touching=updated_data.Cobs_touching; // Update shared data with the updated variables
                        logit( "[Server]:centered pos recived and sent to drone and target");
                        centered = 1;
                        write(server_targets[1],&data,sizeof(data));
                        }
                        logit( "[Server]: max updated");
                        write(server_drone[1],&data,sizeof(data));

                        break;
                    case 1: //keyboard
                    if (centered)
                    {
                        data.key=updated_data.key; // Update shared data with the updated variables
                        write(server_drone[1],&data,sizeof(data));
                        sprintf(msg, "[Server]: key %c recieved and sent to Drone", data.key);
                        logit(msg);
                        /* code */
                    }
                        break;
                    case 2: //drone
                        // double position[6];
                        memcpy(data.drone_pos, updated_data.drone_pos, sizeof(updated_data.drone_pos));
                        sprintf(msg, "[Server]: New pos drone sent to UI %f,%f", data.drone_pos[0],data.drone_pos[1]);
                        logit(msg);
                        data.Cobs_touching=updated_data.Cobs_touching; // Update shared data with the updated variables

                        write(server_UI[1],&data,sizeof(data));
                        write(server_targets[1],&data,sizeof(data));

                        break;
                    case 3: //obstacle
                        logit( "[Server]: obstacles received ");
                        memcpy(data.obstacles, updated_data.obstacles, sizeof(updated_data.obstacles));
                        write(server_obstacles[1],&data,sizeof(data));
                        write(server_drone[1],&data,sizeof(data));
                        write(server_UI[1],&data,sizeof(data));

                        break;
                    case 4: //target
                       logit( "[Server]: Targets received ");
                        memcpy(data.targets, updated_data.targets, sizeof(updated_data.targets));
                        data.targetReached = updated_data.targetReached;
                        write(server_obstacles[1],&data,sizeof(data));
                        write(server_UI[1],&data,sizeof(data));
                        break;
                    default:
                        break;
                    }
                }
            }
        }

    }


    // clean up
    close(server_drone[1]);
    close(server_keyboard[1]);
    close(server_UI[1]);
    close(server_obstacles[1]);
    close(server_targets[1]);
    for(int i=0; i< NUM_PROCESSES-1; i++){
        close(rec_pipes[i][0]);
    }


    return 0;
}
