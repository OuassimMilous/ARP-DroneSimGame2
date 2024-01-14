#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "include/constants.h"
#include <math.h>

double forceX = 0.0, forceY = 0.0;
double repfx = 0.0, repfy = 0.0;
int exit_flag = 0;
struct data data;
FILE *file;
sem_t *LOGsem;
char msg[100];
    int drone_server[2], server_drone[2];

void logit(char *msg) {
    sem_wait(LOGsem);

    file = fopen(LOGPATH, "a");
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

// a function to update the position according to Euler's formula
void calc_position(char key, double position[6]) {

    switch (key) {
        case 'w':
            forceY = forceY + FORCEY; // Move up
            break;
        case 'a':
            forceX = forceX - FORCEX; // Move left
            break;
        case 's':
            forceY = forceY - FORCEY; // Move down
            break;
        case 'd':
            forceX = forceX + FORCEX; // Move right
            break;
        case 'q':
            forceX = forceX - FORCEX; // Move left
            forceY = forceY + FORCEY; // Move up
            break;
        case 'e':
            forceX = forceX + FORCEX; // Move right
            forceY = forceY + FORCEY; // Move up
            break;
        case 'z':
            forceX = forceX - FORCEX; // Move left
            forceY = forceY - FORCEY; // Move down
            break;
        case 'c':
            forceX = forceX + FORCEX; // Move right
            forceY = forceY - FORCEY; // Move down
            break;
        case 'x':
            forceX = 0;
            forceY = 0;
            break;
        case 27:
            exit_flag = 1; // Set exit flag to true
            break;
    }
        double sumx=0,sumy=0;

  for (int i = 0; i < NUM_OBSTACLES; i++) {
     // Check the distance between the drone and the target
            double distance = sqrt(pow(data.drone_pos[0] - data.obstacles[i * 2], 2) +
                                   pow(data.drone_pos[1] - data.obstacles[i * 2 + 1], 2));
            double angle = atan2(data.obstacles[i * 2 +1] - data.obstacles[i * 2], data.drone_pos[1] - data.drone_pos[0]);
            sprintf(msg, "distance %f",distance);
            logit(msg);
            // if it reaches an obstacle, calcilate the repolsive force
            if (distance <= THRESH_TOUCHOBS) {
                data.Cobs_touching += 1;
   
                if(data.obstacles[2*i]>data.drone_pos[4] && forceX>0){
                    sumx -= pow((1/distance)-(1/THRESH_TOUCH), 2)*cos(angle);
                }else if(data.obstacles[2*i]<data.drone_pos[4] && forceX<0){
                    sumx += pow((1/distance)-(1/THRESH_TOUCH), 2)*cos(angle);
                }else if(data.obstacles[2*i+1]>data.drone_pos[5] && forceY>0){
                    sumy -= pow((1/distance)-(1/THRESH_TOUCH), 2)*sin(angle);
                }else if(data.obstacles[2*i+1]<data.drone_pos[5] && forceY<0){
                    sumy += pow((1/distance)-(1/THRESH_TOUCH), 2)*sin(angle);
                }
                repfx = -0.5 * N * sumx;
                repfy = -0.5 * N * sumy;


                if (forceX > 0) {
                    forceX -= fabs(repfx); 
                } else if (forceX < 0) {
                    forceX += fabs(repfx);
                }

                if (forceY > 0) {
                    forceY -= fabs(repfy);
                } else if (forceY < 0) {
                    forceY += fabs(repfy);
                }

                write(drone_server[1], &data, sizeof(data));
                // Log the information
                sprintf(msg, "[Drone]: Drone touched obstacle %d times, this one %d", data.Cobs_touching, i * 2);
                logit(msg);
            }
    }

     // Calculate new positions using the euler's formula
    double newX = (forceX * T * T - M * position[4] + 2 * M * position[2] + K * T * position[0]) / (M + K * T);
    double newY = (forceY * T * T - M * position[5] + 2 * M * position[3] + K * T * position[1]) / (M + K * T);

    if (newX>=data.max[0]){
        newX = data.max[0]-0.5;
        forceX = 0;
    }else if (newX <=0){
        newX = 0.5;
        forceX = 0;
    }
    // update the positions
    position[0] = newX;
    // Shift the old and older positions
    position[2] = position[0];
    position[4] = position[2];
    
    
    if (newY>=data.max[1]){
        newY = data.max[1]-0.5;
        forceY = 0;
    }else if (newY <=0){
        newY = 0.5;
        forceY = 0;
        
    }
  
    // update the positions
    position[1] = newY;
    // Shift the old and older positions
    position[5] = position[3];
    position[3] = position[1];
    

}

int main(int argc, char *argv[]) {
    // Declaration of variables
    char receivedChar;
    double position[6];

    // Open the semaphore for logging
    LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); // Initial value is 1
    if (LOGsem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // PIPES
    sscanf(argv[0], "%d %d|%d %d", &drone_server[0], &drone_server[1], &server_drone[0], &server_drone[1]);
    close(drone_server[0]); // Close unnecessary pipes
    close(server_drone[1]);

    while (exit_flag == 0) {
        read(server_drone[0], &data, sizeof(data));
        calc_position(data.key, data.drone_pos);
        sprintf(msg, "[drone]: key %c received, new pos: %f,%f", data.key, data.drone_pos[0], data.drone_pos[1]);
        logit(msg);
        write(drone_server[1], &data, sizeof(data));
    }

    // Cleanup
    close(server_drone[1]);
    close(drone_server[0]);

    return 0;
}
