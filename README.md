# ARP-DroneSimGame2

## Short Definitions of Active Components:

### Master (master.c)
- **Purpose:** Creates the semaphores, forks the processes, and launches the other active components.
- **Primitives Used:** Semaphores, file operations, fork, pipes.

### Server (server.c)
- **Purpose:** Manages communication between components and logs drone positions.
- **Primitives Used:** Semaphores, file operations, pipes.

### UI (ui.c)
- **Purpose:** Uses ncurses to display the drone position and UI messages.
- **Primitives Used:** Semaphores, ncurses, pipes.

### Keyboard (keyboard.c)
- **Purpose:** Captures keyboard input and sends it to the drone through a pipe.
- **Primitives Used:** Semaphores, pipes.

### Drone (drone.c)
- **Purpose:** Simulates a drone's movement based on keyboard input.
- **Primitives Used:** Semaphores, pipes.

### Targets (targets.c)
- **Purpose:** Creates targets and checks whether they were reached.
- **Primitives Used:** Semaphores, pipes.

### Obstacles (obstacles.c)
- **Purpose:** Creates obstacles and keeps changing their positions periodically.
- **Primitives Used:** Semaphores, pipes.

### Watchdog (watchdog.c)
- **Purpose:** Monitors the status of all processes and logs them.
- **Primitives Used:** Semaphores, pipes.

## List of Components, Directories, Files:

### Directories:
- `include/`: Contains header files.
- `log/`: Contains log files.
- `/`: Contains source code files.

### Files:
- `master.c`: Main program.
- `server.c`: Server component.
- `ui.c`: UI component.
- `keyboard.c`: Keyboard component.
- `drone.c`: Drone component.
- `targets.c`: Targets component.
- `obstacles.c`: Obstacles component.
- `watchdog.c`: Watchdog component.
- `constants.h`: Header file with constant values.
- `Makefile`: Build automation file.
- `README.md`: Documentation file.

## Instructions for Installing and Running:

### Prerequisites:
- Ensure you have the necessary libraries installed:
  - `ncurses`
  - `konsole`

### Installation:
1. Clone the repository.
2. Create the build directory:
   ```sh
   mkdir build
   ```
3. Compile the components using make:
   ```sh
   make
   ```

### Running:
  ```sh
  ./build/master
  ```


