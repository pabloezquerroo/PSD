#include "graph.h"

/**
 * Shows an error message.
 * 
 * @param programName Name of the executable program. 
 * @param msg Error message.
 */
void showError (char* msg, char* programName){	
	printf ("%s\n", msg);
	printf ("Usage: >%s worldWidth worldHeight iterations [step|auto] [outputImage]\n", programName);
	exit (0);
}


int main(int argc, char* argv[]){

	// The window to display the cells
	SDL_Window* window = NULL;

	// The surface where the cells are drawn
	SDL_Surface* screenSurface = NULL;

	// The window renderer
	SDL_Renderer* renderer = NULL;
	
	// World's size
	int worldWidth, worldHeight;
	
	// Screen size
	int screenWidth, screenHeight;
		
	// Number of iterations
	int totalIterations;

	// Output file
	char* outputFile = NULL;

	// The worlds
	unsigned short * worldA;
	unsigned short * worldB;
	
	// Auto mode?
	int autoMode;
	
	// User's input and events
	char ch;
	int isquit = 0;
	SDL_Event event;
	
	// Time
	struct timeval stop, start, totalInit, totalEnd;
		
	
		// Check number of arguments
		if ((argc < 5) || (argc > 6)){
			showError ("Wrong number of parameters!\n", argv[0]);
		}
		
		// Read parameters
		worldWidth = atoi (argv[1]);
		worldHeight = atoi (argv[2]);
		totalIterations = atoi (argv[3]);
		
		if (strcmp(argv[4], "step") == 0)
			autoMode = 0;
		else if (strcmp(argv[4], "auto") == 0)
			autoMode = 1;
		else
			showError ("Wrong mode, please select [step|auto]\n", argv[0]);
					
		if (argc == 6)
			outputFile = argv[5];
				
		srand (SEED);

		// Calculates screen size
		screenWidth = worldWidth * CELL_SIZE;
		screenHeight = worldHeight * CELL_SIZE;
				
		// Show parameters...
		printf ("Executing with:\n");
		printf ("\tWorld size:%d x %d\n", worldWidth, worldHeight);
		printf ("\tScreen size:%d x %d\n", screenWidth, screenHeight);
		printf ("\tNumber of iterations:%d\n", totalIterations);
		printf ("\tExecution mode:%s\n", argv[4]);				
		if (argc == 6)
			printf ("\tOutputFile:[%s]\n", outputFile);
					
		// Create empty worlds
		worldA = (unsigned short*) malloc (worldWidth * worldHeight * sizeof (unsigned short));
		worldB = (unsigned short*) malloc (worldWidth * worldHeight * sizeof (unsigned short));
		clearWorld (worldA, worldWidth, worldHeight);
		clearWorld (worldB, worldWidth, worldHeight);		
				
		// Create a random world		
		initRandomWorld (worldA, worldWidth, worldHeight);
		printf ("World initialized\n");		
		
		// Init video mode	
		if(SDL_Init(SDL_INIT_VIDEO) < 0){
			printf ("Error initializing SDL: %s\n", SDL_GetError());
			exit (-1);
		}
		
		// Create window
		window = SDL_CreateWindow( "Práctica 3 de PSD",
									0, 0,
									screenWidth, screenHeight,
									SDL_WINDOW_SHOWN);			

		// Check if the window has been successfully created
		if( window == NULL ){
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			exit (-1);
		}
		
		// Create a renderer
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
								
		// Clear renderer and draw iteration 0
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(renderer);		
		drawWorld (worldB, worldA, renderer, worldWidth, worldHeight);		
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
		
		// Set timer for the whole execution
		gettimeofday(&totalInit, NULL);

		// Update the world
		for (int iteration=1; iteration<=totalIterations && !isquit; iteration++){
			
			printf ("Processing iteration %d: ", iteration);

			// Clear renderer			
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);			
			
			// Set timer for this iteration
			gettimeofday(&start, NULL);
	
			// Swap worlds between iterations
			if (iteration%2 == 1){					 	
				updateWorld (worldA, worldB, worldWidth, worldHeight);				
				drawWorld (worldA, worldB, renderer, worldWidth, worldHeight);	
				clearWorld (worldA, worldWidth, worldHeight);		
			}
			else{
				updateWorld (worldB, worldA, worldWidth, worldHeight);
				drawWorld (worldB, worldA, renderer, worldWidth, worldHeight);
				clearWorld (worldB, worldWidth, worldHeight);	
			}
			
			// End timer for this iteration
			gettimeofday(&stop, NULL);
			printf ("%f seconds\n", (double) ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec)/1000000.0);			

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
		
		// Set timer for the whole execution
		gettimeofday(&totalEnd, NULL);
		printf ("Total execution time:%f seconds\n", (double) ((totalEnd.tv_sec - totalInit.tv_sec) * 1000000 + totalEnd.tv_usec - totalInit.tv_usec)/1000000.0);
			
		// Save file?
		if (outputFile != NULL)			
			saveImage (renderer, outputFile,screenWidth, screenHeight);				

		// Destroy window		
		SDL_DestroyWindow(window);

	// Game over
	printf ("Game Over!!! Press Enter to continue...");
	ch = getchar();
		
    // Exiting...
    SDL_Quit();

    return 0;
}



