#include "master.h"

void procesoMaster(int worldWidth, int worldHeight, int totalIterations, int distModeStatic, int autoMode, int grain, SDL_Renderer* renderer, SDL_Window* window){
    
	// The surface where the cells are drawn
	SDL_Surface* screenSurface = NULL;
    
    // The worlds
	unsigned short * worldA;
	unsigned short * worldB;

    // User's input and events
	char ch;
	int isquit = 0, hayCataclismo = 0;
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
    printf ("Press Enter to continue...\n");
    ch = getchar();	
    
    // Update the world
    for (int iteration=1; iteration<=totalIterations && !isquit; iteration++){
    	printf ("Processing iteration %d: \n", iteration);

        hayCataclismo = (rand()%100)<PROB_CATACLYSM;
        
        // Clear renderer			
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);		

        // Set timer for this iteration
        iterStartTime = MPI_Wtime();

        if (hayCataclismo){
            for(int i = 0; i < worldHeight; i++){
                worldA[i*worldWidth] = CELL_CATACLYSM;
                worldB[i*worldWidth] = CELL_EMPTY;
                worldA[(i*worldWidth)+worldWidth-1] = CELL_CATACLYSM;
                worldB[(i*worldWidth)+worldWidth-1] = CELL_EMPTY;
            }
        }

        // Swap worlds between iterations
        if (iteration%2 == 1){					 	
            if(distModeStatic == 1)
                staticUpdateWorld (worldA, worldB, worldWidth, worldHeight, hayCataclismo);				
            else			 	
                dynamicUpdateWorld(worldA, worldB, worldWidth, worldHeight, grain, hayCataclismo);	
            drawWorld (worldA, worldB, renderer, 0, worldHeight - 1, worldWidth, worldHeight);	
            clearWorld (worldA, worldWidth, worldHeight);		
        }
        else{
            if(distModeStatic == 1)
                staticUpdateWorld (worldB, worldA, worldWidth, worldHeight, hayCataclismo);				
            else			 	
                dynamicUpdateWorld(worldB, worldA, worldWidth, worldHeight, grain, hayCataclismo);
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

void dynamicUpdateWorld(unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight, int grain, int hayCataclismo){
   
}

void staticUpdateWorld (unsigned short *currentWorld, unsigned short *newWorld, int worldWidth, int worldHeight, int hayCataclismo){
    int size, grain, lineasRecv, filaActual, ultimoGrain, lineasLeidas;
    unsigned short *area, *top, *bottom;
    MPI_Comm_size( MPI_COMM_WORLD , &size);
    int *tablaPunteros = (int*) malloc(size*sizeof(int));
    MPI_Status status;
    grain = worldHeight/(size - 1);

    filaActual = 0;
    area = currentWorld;
    top = currentWorld + (worldWidth*(worldHeight-1));
    bottom = currentWorld + (worldWidth*grain);
    for(int i = 1; i < size-1; i++){
        tablaPunteros[i] = filaActual;
        MPI_Send(&grain, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(top, worldWidth, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);
        MPI_Send(area, worldWidth*grain, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);
        MPI_Send(bottom, worldWidth, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);

        filaActual += grain;
        top = bottom - worldWidth;
        area = bottom;
        bottom = area + (worldWidth*grain); //el la ultima iteracion no vale para nada
    }

    tablaPunteros[size-1] = filaActual;
    bottom = currentWorld;
    ultimoGrain = grain + (worldHeight%grain);

    MPI_Send(&ultimoGrain, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD);
    MPI_Send(top, worldWidth, MPI_UNSIGNED_SHORT, size-1, 0, MPI_COMM_WORLD);
    MPI_Send(area, worldWidth*grain, MPI_UNSIGNED_SHORT, size-1, 0, MPI_COMM_WORLD);
    MPI_Send(bottom, worldWidth, MPI_UNSIGNED_SHORT, size-1, 0, MPI_COMM_WORLD);

    while(lineasLeidas<worldHeight){
        MPI_Recv(&lineasRecv, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(newWorld + ((tablaPunteros[status.MPI_SOURCE])*worldWidth), worldWidth*lineasRecv, MPI_UNSIGNED_SHORT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
        lineasLeidas+=lineasRecv;
    }

    free(tablaPunteros);
}

void endWorkers(){
    int size;
    int fin = 0;
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    for(int i = 1; i<size; ++i)
        MPI_Send(&fin, 1,MPI_INT,i,0,MPI_COMM_WORLD);
}