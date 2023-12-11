#include "graph.h"
#include "mpi.h"

// Enables/Disables the log messages from the master process
#define DEBUG_MASTER 1

// Probability that a cataclysm may occur [0-100] :(
#define PROB_CATACLYSM 10

// Number of iterations between two possible cataclysms
#define ITER_CATACLYSM 5

//Functions

void procesoMaster(int worldWidth, int worldHeight, int totalIterations, int distModeStatic, int autoMode, int grain, SDL_Renderer* renderer, SDL_Window* window);

void dynamicUpdateWorld(unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight, int grain, int hayCataclismo);

void staticUpdateWorld(unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight, int hayCataclismo);
