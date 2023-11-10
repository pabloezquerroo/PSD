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
	game->player1Bet = DEFAULT_BET;
	game->player2Bet = DEFAULT_BET;
	game->player1Stack = INITIAL_STACK;
	game->player2Stack = INITIAL_STACK;
	
	// Game status variables
	game->endOfGame = FALSE;
	game->status = gameEmpty;


    // Initialize mutexes and variable conditions
    pthread_mutex_init(&game->cerrojoTurno, NULL);
    pthread_cond_init(&game->condTurno, NULL);
    //pthread_mutex_init(&games[i].players_mutex, NULL);

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

void recibeCartas(blackJackns__tDeck *playerDeck, int gameId, int numCartas){
        for(int i=0; i<numCartas; i++){
                playerDeck->cards[playerDeck->__size]=getRandomCard(&games[gameId].gameDeck);
                (playerDeck->__size)++;
        }
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

int buscaJugador(xsd__string playerName){
        int i = 0;
        int jugExistente=FALSE;
        int gameIndex=ERROR_PLAYER_NOT_FOUND;
        while(i < MAX_GAMES && !jugExistente){
            // Tries to locate player in game i
            if ((strcmp(games[i].player1Name, playerName)==0)  || (strcmp(games[i].player2Name, playerName)==0)){
                    jugExistente = TRUE;
                    gameIndex=i;
            }
            i++;
        }
        return gameIndex;
}

int buscaHueco(){
    int i = 0;
    int hayHueco =FALSE;
	int gameIndex = ERROR_SERVER_FULL;
	// For every game
	while(i < MAX_GAMES && !hayHueco){
		if(games[i].status != gameReady){
			hayHueco = TRUE;
            gameIndex=i;
        }
        i++;
	}
    return gameIndex;
}

int blackJackns__register (struct soap *soap, blackJackns__tMessage playerName, int* result){
		
	int gameIndex;

    // Set \0 at the end of the string
    playerName.msg[playerName.__size] = 0;
    if (DEBUG_SERVER)
            printf ("[Register] Registering new player -> [%s]\n", playerName.msg);

    gameIndex= buscaHueco();
    
    if(gameIndex == ERROR_SERVER_FULL){
        printf("Servidor lleno, no se puede registrar\n");
        *result= ERROR_SERVER_FULL;
    }
    else{
        if(buscaJugador(playerName.msg)==ERROR_PLAYER_NOT_FOUND){ //jugador no existe, se puede registrar       
            if(games[gameIndex].status==gameEmpty){
                strcpy(games[gameIndex].player1Name, playerName.msg);
                games[gameIndex].status=gameWaitingPlayer;
                
            }else if(games[gameIndex].status==gameWaitingPlayer){
                strcpy(games[gameIndex].player2Name, playerName.msg);
                games[gameIndex].status=gameReady;
            }
            *result=gameIndex;
        }else{ //jugador ya existente
            printf("Jugador existente, no se puede registrar\n");
            *result=ERROR_NAME_REPEATED;
        }
    }
    
  	return SOAP_OK;
}

int jugador1o2(int gameIndex, xsd__string playerName){
    int jug=-1;
    if (strcmp (games[gameIndex].player1Name, playerName) == 0)
        jug = player1;
    else if(strcmp (games[gameIndex].player2Name, playerName) == 0)
        jug = player2;
    return jug;
}

unsigned int compruebaGanadorMano(blackJackns__tDeck *deckP1, blackJackns__tDeck *deckP2){
    unsigned int puntosP1, puntosP2;
    puntosP1= calculatePoints(deckP1);
    puntosP2= calculatePoints(deckP2);
    if (puntosP1>0 && puntosP2>0){
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
        }else{
                return -1;
        }
    }
    return -1;
}

int compruebaFin(unsigned int *stackA, unsigned int *stackB){
        if (*stackA==0)
                return player2;
        else if (*stackB==0)
                return player1;
        else 
                return -1;
}


int blackJackns__getStatus(struct soap *soap, int gameId, blackJackns__tMessage playerName, blackJackns__tBlock *status){
    int ganador;
    int ganadorMano;
    int numJugador;
    char *mensaje;
    
    allocClearBlock(soap, status);
    mensaje=malloc(sizeof(char)*STRING_LENGTH);

    playerName.msg[playerName.__size] = 0;
    numJugador=jugador1o2(gameId,playerName.msg);



    ganador= compruebaFin(&games[gameId].player1Stack, &games[gameId].player2Stack);
    if (ganador!=-1){
        games[gameId].endOfGame=TRUE;
        if(numJugador==player1){
            if(ganador==player1){
                copyGameStatusStructure(status, "Has ganado la partida", &games[gameId].player1Deck, GAME_WIN);

            }else{
                copyGameStatusStructure(status, "Has perdido la partida", &games[gameId].player1Deck, GAME_LOSE);
            }
        }else{
            if(ganador==player2){
                copyGameStatusStructure(status, "Has ganado la partida", &games[gameId].player2Deck, GAME_WIN);

            }else{
                copyGameStatusStructure(status, "Has perdido la partida", &games[gameId].player2Deck, GAME_LOSE);
            }
        }
    }else{//juego no ha acabado
        if((strcmp(playerName.msg, games[gameId].player1Name)==0 && games[gameId].currentPlayer!=player1) ||
        (strcmp(playerName.msg, games[gameId].player2Name)==0 && games[gameId].currentPlayer!=player2)){ //no es mi turno
            pthread_cond_wait(&games[gameId].condTurno, &games[gameId].cerrojoTurno);
        }

        pthread_mutex_lock(&games[gameId].cerrojoTurno);
        ganadorMano=compruebaGanadorMano(&games[gameId].player1Deck, &games[gameId].player2Deck);
        if(ganadorMano!=-1){
            if(numJugador==player1){
                if(ganadorMano==player1){
                    games[gameId].player1Stack+=games[gameId].player2Bet;

                    sprintf(mensaje, "\nHas ganado la mano, ganas %d monedas.\n", games[gameId].player2Bet);
                    copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_WAIT); 
                }
                else{
                    games[gameId].player1Stack-=games[gameId].player1Bet;

                    sprintf(mensaje, "\nHas perdido la mano, pierdes %d monedas.\n", games[gameId].player1Bet);
                    copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_WAIT); 
                }
            }else{
                if(ganadorMano==player1){
                    games[gameId].player2Stack-=games[gameId].player2Bet;

                    sprintf(mensaje, "\nHas perdido la mano, pierdes %d monedas.\n", games[gameId].player2Bet);
                    copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_WAIT); 
                }
                else{
                    games[gameId].player2Stack+=games[gameId].player1Bet;

                    sprintf(mensaje, "\nHas ganado la mano, ganas %d monedas.\n", games[gameId].player1Bet);
                    copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_WAIT); 
                }
            }
            games[gameId].currentPlayer=calculateNextPlayer(games[gameId].currentPlayer);
            pthread_mutex_unlock(&games[gameId].cerrojoTurno);
            pthread_cond_broadcast(&games[gameId].condTurno);
        }else{
            if(games[gameId].currentPlayer==numJugador){ //turno del jugador

                if (numJugador==player1){
                    recibeCartas(&games[gameId].player1Deck, gameId, 2);

                    sprintf(mensaje, "Es tu turno, tienes %d puntos.\n", calculatePoints(&games[gameId].player1Deck));
                    copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_PLAY); 
                }else{
                    recibeCartas(&games[gameId].player2Deck, gameId, 2);

                    sprintf(mensaje, "Es tu turno, tienes %d puntos.\n", calculatePoints(&games[gameId].player2Deck));
                    copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_PLAY);
                }
            }
        }
    }
    return SOAP_OK;
} 

int blackJackns__playermove(struct soap *soap, int gameId, blackJackns__tMessage playerName, unsigned int playerMove, blackJackns__tBlock* status){
    char* mensaje;
    int numJugador=jugador1o2(gameId, playerName.msg);
    allocClearBlock(soap, status);
    mensaje=malloc(sizeof(char)*STRING_LENGTH);

    switch (playerMove)
    {
    case PLAYER_HIT_CARD:
    if(numJugador==player1){
        recibeCartas(&games[gameId].player1Deck, gameId, 1);

        if(calculatePoints(&games[gameId].player1Deck)>GOAL_GAME){ // Se ha pasado
            sprintf(mensaje, "\nTienes %d puntos, te has pasado.\n", calculatePoints(&games[gameId].player1Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_WAIT);
                
            games[gameId].currentPlayer=calculateNextPlayer(games[gameId].currentPlayer);

            pthread_mutex_unlock(&games[gameId].cerrojoTurno);
            pthread_cond_broadcast(&games[gameId].condTurno);

        }else{ //puede seguir jugando
            sprintf(mensaje, "Puedes seguir jugando, tienes %d puntos.\n", calculatePoints(&games[gameId].player1Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_PLAY);
        }
    }else{
        recibeCartas(&games[gameId].player2Deck, gameId, 1);

        if(calculatePoints(&games[gameId].player2Deck)>GOAL_GAME){ // Se ha pasado
            sprintf(mensaje, "\nTienes %d puntos, te has pasado.\n", calculatePoints(&games[gameId].player2Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_WAIT);

            games[gameId].currentPlayer=calculateNextPlayer(games[gameId].currentPlayer);

            pthread_mutex_unlock(&games[gameId].cerrojoTurno);
            pthread_cond_broadcast(&games[gameId].condTurno);

        }else{ //puede seguir jugando
            sprintf(mensaje, "Puedes seguir jugando, tienes %d puntos.\n", calculatePoints(&games[gameId].player2Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_PLAY);
        }
    }
    break;
    case PLAYER_STAND:
        if(numJugador==player1){
            sprintf(mensaje, "\nTe plantas con %d puntos.\n", calculatePoints(&games[gameId].player1Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player1Deck, TURN_WAIT);
        }else{
            sprintf(mensaje, "\nTe plantas con %d puntos.\n", calculatePoints(&games[gameId].player2Deck));
            copyGameStatusStructure(status, mensaje, &games[gameId].player2Deck, TURN_WAIT);
        }
        games[gameId].currentPlayer=calculateNextPlayer(games[gameId].currentPlayer);

        pthread_mutex_unlock(&games[gameId].cerrojoTurno);
        pthread_cond_broadcast(&games[gameId].condTurno);

        break;
    default:
        break;
    }

    return SOAP_OK;
}

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