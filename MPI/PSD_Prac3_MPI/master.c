#include "master.h"

void procesoMaster(int worldWidth, int worldHeight, int totalIterations, int distModeStatic, int autoMode, int grain, SDL_Renderer* renderer, SDL_Window* window){
    
	// The surface where the cells are drawn
	SDL_Surface* screenSurface = NULL;
    
    // The worlds
	unsigned short * worldA;
	unsigned short * worldB;

    // User's input and events
	char ch;
	int isquit = 0;
	SDL_Event event;

	// Time
	double iterStopTime, iterStartTime;

    // Create empty worlds
    worldA = (unsigned short*) malloc (worldWidth * worldHeight * sizeof (unsigned short));
    worldB = (unsigned short*) malloc (worldWidth * worldHeight * sizeof (unsigned short));
    clearWorld (worldA, worldWidth, worldHeight);
    clearWorld (worldB, worldWidth, worldHeight);	

    // Create a random world		
    initRandomWorld (worldA, worldWidth, worldHeight);
    printf ("World initialized\n");		

    // Clear renderer and draw iteration 0
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);		
    drawWorld (worldB, worldA, renderer, 0, worldHeight - 1, worldWidth, worldHeight);		
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
				
    // Read event
    if (SDL_PollEvent(& event)) 
        if (event.type == SDL_QUIT) 
            isquit = 1;

    // Stop in iteration 0
    printf ("World created (iteration 0)\n");		
    printf ("Press Enter to continue...");
    ch = getchar();	
    
    // Update the world
    for (int iteration=1; iteration<=totalIterations && !isquit; iteration++){
    	printf ("Processing iteration %d: ", iteration);

        // Clear renderer			
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);		

        // Set timer for this iteration
        iterStartTime = MPI_Wtime();

        // Swap worlds between iterations
        if (iteration%2 == 1){					 	
            if(distModeStatic == 1)
                staticUpdateWorld (worldA, worldB, worldWidth, worldHeight);				
            else			 	
                dynamicUpdateWorld(worldA, worldB, worldWidth, worldHeight, grain);	
            drawWorld (worldA, worldB, renderer, 0, worldHeight - 1, worldWidth, worldHeight);	
            clearWorld (worldA, worldWidth, worldHeight);		
        }
        else{
            if(distModeStatic == 1)
                staticUpdateWorld (worldB, worldA, worldWidth, worldHeight);				
            else			 	
                dynamicUpdateWorld(worldB, worldA, worldWidth, worldHeight, grain);
            drawWorld (worldB, worldA, renderer, 0, worldHeight - 1, worldWidth, worldHeight);
            clearWorld (worldB, worldWidth, worldHeight);	
        }
        
        // End timer for this iteration
        iterStopTime = MPI_Wtime();
        printf("Iteration: %d, Time: %f...\n",iteration,iterStopTime-iterStartTime);

        //Update the surface
        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);					
        
        // Read event
        if (SDL_PollEvent(& event)) 
            if (event.type == SDL_QUIT) 
                isquit = 1;			
        
        if (!autoMode){
            printf ("Press Enter to continue...");
            ch = getchar();
        }		
    }
}

void dynamicUpdateWorld(unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight, int grain){
    
}

void staticUpdateWorld (unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight){

}
