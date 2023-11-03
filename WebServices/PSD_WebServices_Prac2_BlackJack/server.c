#include "server.h"

/** Shared array that contains all the games. */
tGame games[MAX_GAMES];

void initGame (tGame *game){

	// Init players' name
	memset (game->player1Name, 0, STRING_LENGTH);	
	memset (game->player2Name, 0, STRING_LENGTH);

	// Alloc memory for the decks		
	clearDeck (&(game->player1Deck));	
	clearDeck (&(game->player2Deck));	
	initDeck (&(game->gameDeck));
	
	// Bet and stack
	game->player1Bet = 0;
	game->player2Bet = 0;
	game->player1Stack = INITIAL_STACK;
	game->player2Stack = INITIAL_STACK;
	
	// Game status variables
	game->endOfGame = FALSE;
	game->status = gameEmpty;
}

void initServerStructures (struct soap *soap){

	if (DEBUG_SERVER)
		printf ("Initializing structures...\n");

	// Init seed
	srand (time(NULL));

	// Init each game (alloc memory and init)
	for (int i=0; i<MAX_GAMES; i++){
		games[i].player1Name = (xsd__string) soap_malloc (soap, STRING_LENGTH);
		games[i].player2Name = (xsd__string) soap_malloc (soap, STRING_LENGTH);
		allocDeck(soap, &(games[i].player1Deck));	
		allocDeck(soap, &(games[i].player2Deck));	
		allocDeck(soap, &(games[i].gameDeck));
		initGame (&(games[i]));
	}	
}

void initDeck (blackJackns__tDeck *deck){

	deck->__size = DECK_SIZE;

	for (int i=0; i<DECK_SIZE; i++)
		deck->cards[i] = i;
}

void clearDeck (blackJackns__tDeck *deck){

	// Set number of cards
	deck->__size = 0;

	for (int i=0; i<DECK_SIZE; i++)
		deck->cards[i] = UNSET_CARD;
}

tPlayer calculateNextPlayer (tPlayer currentPlayer){
	return ((currentPlayer == player1) ? player2 : player1);
}

unsigned int getRandomCard (blackJackns__tDeck* deck){

	unsigned int card, cardIndex, i;

		// Get a random card
		cardIndex = rand() % deck->__size;
		card = deck->cards[cardIndex];

		// Remove the gap
		for (i=cardIndex; i<deck->__size-1; i++)
			deck->cards[i] = deck->cards[i+1];

		// Update the number of cards in the deck
		deck->__size--;
		deck->cards[deck->__size] = UNSET_CARD;

	return card;
}

void copyGameStatusStructure (blackJackns__tBlock* status, char* message, blackJackns__tDeck *newDeck, int newCode){

	// Copy the message
	memset((status->msgStruct).msg, 0, STRING_LENGTH);
	strcpy ((status->msgStruct).msg, message);
	(status->msgStruct).__size = strlen ((status->msgStruct).msg);

	// Copy the deck, only if it is not NULL
	if (newDeck->__size > 0)
		memcpy ((status->deck).cards, newDeck->cards, DECK_SIZE*sizeof (unsigned int));	
	else
		(status->deck).cards = NULL;

	(status->deck).__size = newDeck->__size;

	// Set the new code
	status->code = newCode;	
}

unsigned int calculatePoints (blackJackns__tDeck *deck){

	unsigned int points = 0;
		
		for (int i=0; i<deck->__size; i++){

			if (deck->cards[i] % SUIT_SIZE < 9)
				points += (deck->cards[i] % SUIT_SIZE) + 1;
			else
				points += FIGURE_VALUE;
		}

	return points;
}

int nombreValido(xsd__string nombre){
            // For every game
    int i = -1;
    int nombreExistente = FALSE;
    int serverLleno = FALSE;
    int partidaDisponible = FALSE, numPartida;

    while(i < MAX_GAMES && !nombreExistente){
        // Tries to locate player in game i
        i++;
        if (strcmp (games[i].player1Name, nombre) == 0  || (games[i].player2Name, nombre) == 0){
                nombreExistente = TRUE;
        }else if(games[i].status!=gameReady){
                partidaDisponible = TRUE;
                numPartida=i;
        }else if(i==MAX_GAMES-1){
                serverLleno = TRUE;
        }
    }
    if(nombreExistente){
        printf("Nombre %s ya existente, no es posible registrarse\n", nombre);
        return ERROR_NAME_REPEATED;
    }else if(serverLleno){
        printf("Nombre %s ya existente, no es posible registrarse\n", nombre);
        return ERROR_SERVER_FULL;
    }
    else if(partidaDisponible)
        return numPartida;
}

int blackJackns__register (struct soap *soap, blackJackns__tMessage playerName, int* result){
		
	int gameIndex;

        // Set \0 at the end of the string
        playerName.msg[playerName.__size] = 0;

        if (DEBUG_SERVER)
                printf ("[Register] Registering new player -> [%s]\n", playerName.msg);

        gameIndex= nombreValido(playerName.msg);
	
  	return SOAP_OK;
}

int blackJackns__getStatus(struct soap *soap, blackJackns__tMessage playerName, int* result){
        return SOAP_OK;
}



/////// MARTIN ////////

unsigned int calculatePoints (blackJackns__tDeck *deck){
    unsigned int points;
        // Init...
        points = 0;

        for (int i=0; i<deck->__size; i++){

            if (deck->cards[i] % SUIT_SIZE < 9)
                points += (deck->cards[i] % SUIT_SIZE) + 1;
            else
                points += FIGURE_VALUE;
        }
    return points;
}
int compruebaFin(unsigned int *stackA, unsigned int *stackB){
        if (*stackA==0)
                return player2;
        else if (*stackB==0)
                return player1;
        else 
                return -1;
}

unsigned int compruebaGanadorMano(blackJackns__tDeck *deckP1, blackJackns__tDeck *deckP2){
        unsigned int puntosP1, puntosP2;
        puntosP1= calculatePoints(deckP1);
        puntosP2= calculatePoints(deckP2);
        if ((puntosP1==puntosP2) || (puntosP1>GOAL_GAME && puntosP2>GOAL_GAME)) //empate
                return -1;
        else if(puntosP1>puntosP2 && puntosP1<=GOAL_GAME){
                return player1;
        }else if(puntosP2>puntosP1 && puntosP2<=GOAL_GAME){
                return player2;
        }else if(puntosP1>GOAL_GAME && puntosP2<=GOAL_GAME){
                return player2;
        }else if(puntosP2>GOAL_GAME && puntosP1<=GOAL_GAME){
                return player1;
        }
}
////////////////////////////////////////////////////////

// Busca en que partida está el jugador o si esta en alguna.
int locateGameForPlayer (xsd__string name){
    // For every game
    int i = -1;
    int found = FALSE;
    while(i < MAX_GAMES && !found){
        // Tries to locate player in game i
        i++;
        if (strcmp (games[i].player1Name, name) == 0  || (games[i].player2Name, name) == 0){
            found = TRUE;
        }
    }
    // Return valid game
    if(found){
        return i;
    }else{
        //The player isn't in any current game
        return ERROR_PLAYER_NOT_FOUND;
    }
    
}

int jugador1o2(int numGame, xsd__string nombre){
    if (strcmp (games[numGame].player1Name, nombre) == 0)
        return 0;
    else if(strcmp (games[numGame].player2Name, nombre) == 0)
        return 1;
    return -1;
}


// Service to get current game status
int getStatus (struct soap *soap, blackJackns__tMessage nombre , blackJackns__tBlock * status){

    int numeroPartida, isWinner;
    int ganadorMano;
    int puntosJug;
    char mensaje [STRING_LENGTH];
        // Set \0 at the end of the string
        nombre.msg[nombre.__size] = 0;
        isWinner = FALSE;

        // Allocate memory for the status structure
        allocClearBlock(soap,status);

        if (DEBUG_SERVER)
            printf ("[getStatus] request from -> [%s]\n", nombre.msg);
        // Locate the game
        numeroPartida = locateGameForPlayer(nombre.msg);
        // Player not found...
        if (numeroPartida == ERROR_PLAYER_NOT_FOUND){
            // Allocate memory and copy message into status struct
            copyGameStatusStructure (status, "Player is not currently active in the system! Please, register again...\n", NULL, ERROR_PLAYER_NOT_FOUND);
            printf ("[getStatus] - Player %s not found!\n", nombre.msg);
        }

        // Player is already registered...
        else{
            // Locates player in game
            //pthread_mutex_lock(&games[numeroPartida].players_mutex);//<--------------------------------------
            int jug = jugador1o2(numeroPartida ,nombre.msg);
            //pthread_mutex_unlock(&games[numeroPartida].players_mutex);//<--------------------------------------
            // If game has finish
            if(games[numeroPartida].endOfGame){
                //pthread_mutex_lock(&games[numeroPartida].turn_mutex);//<--------------------------------------
                // Checks if a player won
                ganadorMano=compruebaGanadorMano(&games[numeroPartida].player1Deck, &games[numeroPartida].player2Deck);
                if(ganadorMano != -1){
                    if(ganadorMano==jug){
                        sprintf(mensaje, "You win!\n");
                        copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, GAME_WIN);
                    }else{
                        sprintf(mensaje, "You Lose!\n");
                        copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, GAME_LOSE);
                    }
                }else {
                    sprintf(mensaje, "Draw!\n");
                    copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, GAME_WIN);
                }
                
            //pthread_mutex_unlock(&games[gameIndex].turn_mutex);//<--------------------------------------
            }
            // If game hasn't finsish
            else{
                // If is not player turn waits until unlocked
                if ((strcmp(nombre.msg, games[numeroPartida].player1Name) == 0 && games[numeroPartida].currentPlayer != player1) || 
                    (strcmp(nombre.msg, games[numeroPartida].player2Name) == 0 && games[numeroPartida].currentPlayer != player2)){
                        // Waits for condition
                        //pthread_cond_wait(&games[numeroPartida].turn_cond, &games[numeroPartida].turn_mutex);//<--------------------------------------
                        // Unlocks
                        //pthread_mutex_unlock(&games[numeroPartida].turn_mutex);//<--------------------------------------
                    }
                    
                // If is player turn
                if(games[numeroPartida].currentPlayer == jug){
                    // Locks board and game status
                    //pthread_mutex_lock(&games[numeroPartida].turn_mutex);//<--------------------------------------
                    if(jug=player1){
                        puntosJug=calculatePoints(&games[numeroPartida].player1Deck);
                        sprintf(mensaje, "\nEs tu turno, tienes %d puntos.", puntosJug);
                        copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, TURN_PLAY);
                    }else{
                        puntosJug=calculatePoints(&games[numeroPartida].player2Deck);
                        sprintf(mensaje, "\nEs tu turno, tienes %d puntos.", puntosJug);
                        copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, TURN_PLAY);
                    }
                }
                // If isn't player turn
                else{
                    sprintf(mensaje, "Your rival is thinking...\n");
                    copyGameStatusStructure(status, mensaje, &games[numeroPartida].gameDeck, TURN_WAIT);
                }
            }
        }
    return SOAP_OK;
}

///////////////////////////////////////////////////////////////////////////

void *processRequest(void *soap){

	pthread_detach(pthread_self());

	printf ("Processing a new request...");

	soap_serve((struct soap*)soap);
	soap_destroy((struct soap*)soap);
	soap_end((struct soap*)soap);
	soap_done((struct soap*)soap);
	free(soap);

	return NULL;
}

int main(int argc, char **argv){ 

	struct soap soap;
	struct soap *tsoap;
	pthread_t tid;
	int port;
	SOAP_SOCKET m, s;

		// Check arguments
		if (argc !=2) {
			printf("Usage: %s port\n",argv[0]);
			exit(0);
		}

	// Init soap environment
        soap_init(&soap);

        // Configure timeouts
        soap.send_timeout = 60; // 60 seconds
        soap.recv_timeout = 60; // 60 seconds
        soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
        soap.max_keep_alive = 100; // max keep-alive sequence

        initServerStructures(&soap);

        // Get listening port
        port = atoi(argv[1]);

        // Bind
        m = soap_bind(&soap, NULL, port, 100);

        if (!soap_valid_socket(m)){
                exit(1);
        }

        printf("Server is ON!\n");


		while (TRUE){

			// Accept a new connection
			s = soap_accept(&soap);

			// Socket is not valid :(
			if (!soap_valid_socket(s)){

				if (soap.errnum){
					soap_print_fault(&soap, stderr);
					exit(1);
				}

				fprintf(stderr, "Time out!\n");
				break;
			}

			// Copy the SOAP environment
			tsoap = soap_copy(&soap);

			if (!tsoap){
				printf ("SOAP copy error!\n");
				break;
			}

			// Create a new thread to process the request
			pthread_create(&tid, NULL, (void*(*)(void*))processRequest, (void*)tsoap);
		}

	// Detach SOAP environment
	soap_done(&soap);


	return 0;
}