#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include "include/constants.h"
#include <stdbool.h>
#include <time.h>
bool game_over = false;
time_t start_time;
struct data data;
double time_needed;
int Score;

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



void start_timer() {
    time(&start_time);
}

double get_elapsed_time() {
    time_t end_time;
    time(&end_time);
    return difftime(end_time, start_time);
}


int main(int argc, char const *argv[]) {
    // Initialize ncurses
    initscr();
    cbreak();  // Line buffering disabled
    keypad(stdscr, TRUE);  // Enable special keys
    curs_set(0); // Don't display cursor

    // Create or open a semaphore for logging
    LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); 
    if (LOGsem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }


     // PIPES    
    int  UI_server[2], server_UI[2];
    sscanf(argv[1], "%d %d|%d %d",  &UI_server[0], &UI_server[1], &server_UI[0], &server_UI[1]);
    close(UI_server[0]); //Close unnecessary pipes
    close(server_UI[1]);


    // Get window dimensions
    double height, width;
    getmaxyx(stdscr, height, width);

    //center the drone
    double position[6] = {width / 2, height / 2, width / 2, height / 2, width / 2, height / 2};

    memcpy(data.drone_pos, position, sizeof(position));
    data.max[0] =  width;
    data.max[1] =  height;
    data.Cobs_touching = 0;
    write(UI_server[1], &data, sizeof(data));
    logit("[UI]: Window created and pos sent to server");
    start_timer();
    
    while (1) {
        do
        {
        read(server_UI[0], &data, sizeof(data)); // recieve the updated data
        } while (data.obstacles[0]==0);

        // calculating the score  
        Score = ( 5 * NUM_TARGETS + (2 * NUM_OBSTACLES) + (3 * data.targetReached) - (data.Cobs_touching  * 2) - (get_elapsed_time() * 1.5));

        // Clear the screen
        clear();

        int newHeight, newWidth;
        getmaxyx(stdscr, newHeight, newWidth);

        if (newHeight != data.max[1] || newWidth != data.max[0]) {
            // Screen has been resized, update dimensions and send to the server
            data.max[1] = newHeight;
            data.max[0] = newWidth;
            write(UI_server[1], &data, sizeof(data));
        }
        // Draw the bordered box
        box(stdscr, 0, 0);

        // checking if the user have won
        bool all_targets_hit = true;
        for (int i = 0; i < NUM_TARGETS * 2; ++i) {
            if (data.targets[i] != -1) {
                all_targets_hit = false;
                break;
            }
        }
        if (all_targets_hit) {
            // All targets hit, print GAME OVER and score
            if (!game_over)
            {
                time_needed = get_elapsed_time();
            }
                        Score = ( 5 * NUM_TARGETS + (2 * NUM_OBSTACLES) + (3 * data.targetReached) - (data.Cobs_touching * 2) - (time_needed * 0.15));
                       mvprintw(data.max[0] / 2, data.max[1] / 2 - 8, "GAME OVER, SCORE: %d", Score);
            refresh();
            game_over = true;
        } else {

            // Draw the item at its current position
            mvprintw((int)data.drone_pos[1], (int)data.drone_pos[0], "+");

            // Print the X coordinate
            mvprintw(1, 1, "X: %.2f", data.drone_pos[0]);

            // Print the Y coordinate 
            mvprintw(2, 1, "Y: %.2f", data.drone_pos[1]);

            mvprintw(3, 1, "score: %d", Score);


            // Print targets
            for (int i = 0; i < NUM_TARGETS * 2; i += 2) {
                int x = (int)data.targets[i];
                int y = (int)data.targets[i+1];
            
                mvprintw(y, x, "%d", i / 2);

                sprintf(msg, "[UI]: TARGET x[%d]: %d, y[%d]: %d", i/2, x, i/2, y);
                logit(msg);
            }

            for (int i = 0; i < NUM_OBSTACLES * 2; i += 2) {
                int x = (int)data.obstacles[i];
                int y = (int)data.obstacles[i+1];
            
                mvprintw(y, x, "X");

                sprintf(msg, "[UI]: OBSTACLE x[%d]: %d, y[%d]: %d", i/2, x, i/2, y);
                logit(msg);
            }

            // Refresh the screen
            refresh();
        }
    }

    // End ncurses
    endwin();

    close(server_UI[1]);
    close(UI_server[0]); 


    return 0;
}
