#include "worker.h"

void procesoWorker(int worldWidth, int worldHeight){
    int grain;
    unsigned short *currentWorld, *newWorld;
    MPI_Status status;
    MPI_Recv(&grain, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);    
    while(grain != 0){
        currentWorld = (unsigned short*) malloc (worldWidth * (grain+2) * sizeof (unsigned short));
        newWorld = (unsigned short*) malloc (worldWidth * (grain+2) * sizeof (unsigned short));
        MPI_Recv(currentWorld, worldWidth, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(currentWorld+worldWidth, grain*worldWidth, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(currentWorld+(worldWidth*(grain+1)), worldWidth, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, &status);

        if(DEBUG_WORKER){
            printf("Worker %d: Recibido %d filas\n", status.MPI_SOURCE, grain);
            printf("Worker %d: Recibido top\n", status.MPI_SOURCE);
            printf("Worker %d: Recibido area\n", status.MPI_SOURCE);
            printf("Worker %d: Recibido bottom\n", status.MPI_SOURCE);
            printf("Worker %d: Recibido hayCataclismo\n", status.MPI_SOURCE);
        }
        
        updateWorld(currentWorld, newWorld, worldWidth, grain+2);

        MPI_Send(&grain, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        MPI_Send(newWorld+worldWidth,grain*worldWidth, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD);
        MPI_Recv(&grain, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);  
    }

}