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

    //add
    game->currentPlayer = player1;
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

void recibeCartas(blackJackns__tDeck *playerDeck, blackJackns__tDeck *gameDeck, int numCartas){
        for(int i=0; i<numCartas; i++){
                playerDeck->cards[playerDeck->__size]=getRandomCard(gameDeck);
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
    if(nombreExistente)
        return ERROR_NAME_REPEATED;
    else if(partidaDisponible)
        return numPartida;
    else if(serverLleno)
        return ERROR_SERVER_FULL;
}

int buscaJugador(xsd__string playerName){
        int i = -1;
        int jugExistente=FALSE;
        int gameIndex=ERROR_PLAYER_NOT_FOUND;
        while(i < MAX_GAMES && !jugExistente){
            // Tries to locate player in game i
            i++;
            if (strcmp (games[i].player1Name, playerName) == 0  || (games[i].player2Name, playerName) == 0){
                    jugExistente = TRUE;
                    gameIndex=i;
            }
        }
        return gameIndex;
}

buscaHueco(){
    int i = -1;
    int hayHueco =FALSE;
	int gameIndex = ERROR_SERVER_FULL;
	// For every game
	while(i < MAX_GAMES && !hayHueco){
        i++;
		// Locks game i
		//pthread_mutex_lock(&games[i].players_mutex);//<--------------------------------------
		// If is an empty game
		if(games[i].status != gameReady){
			hayHueco = TRUE;
            gameIndex=i;
        }
		//pthread_mutex_unlock(&games[i].players_mutex);//<--------------------------------------
	}
	// return valid game
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
        result=ERROR_SERVER_FULL;
    }
    else{
        if(buscaJugador(playerName.msg)==ERROR_PLAYER_NOT_FOUND){ //jugador no existe, se puede registrar       
            //mutex
            if(games[gameIndex].status==gameEmpty){
                strcpy(games[gameIndex].player1Name, playerName.msg);
            }else if(games[gameIndex].status==gameWaitingPlayer){
                strcpy(games[gameIndex].player2Name, playerName.msg);
                games[gameIndex].status=gameReady;
            }
            result=gameIndex;
        }else{ //jugador ya existente
            printf("Jugador existente, no se puede registrar\n");
            result=ERROR_NAME_REPEATED;
        }
    }
    
  	return SOAP_OK;
}

int jugador1o2(int gameIndex, xsd__string playerName){
    int jug=-1;
    if (strcmp (games[gameIndex].player1Name, playerName) == 0)
        jug = 0;
    else if(strcmp (games[gameIndex].player2Name, playerName) == 0)
        jug = 1;
    return jug;
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

int compruebaFin(unsigned int *stackA, unsigned int *stackB){
        if (*stackA==0)
                return player2;
        else if (*stackB==0)
                return player1;
        else 
                return -1;
}

int blackJackns__getStatus(struct soap *soap, blackJackns__tMessage playerName, blackJackns__tBlock *status, int* result){
    int ganadorMano=-1;
    int ganador=-1;
    int starterPlayer=player1;
    int numJugador;
    int gameIndex;
    xsd__string mensajeGana, mensajePierde, mensajeEmpata, mensaje;

    gameIndex = buscaJugador(playerName.msg);
    if(gameIndex==ERROR_PLAYER_NOT_FOUND){

        copyGameStatusStructure(status, "Jugador no encontrado\n", NULL, ERROR_PLAYER_NOT_FOUND);

    }else{
        //comprobamos quien ha ganado la mano
        ganadorMano=compruebaGanadorMano(&games[gameIndex].player1Deck, &games[gameIndex].player2Deck);

        if (numJugador==ganadorMano){ // Ganador de la mano
            if(numJugador=player1){ // Jugador 1

                games[gameIndex].player1Stack+= games[gameIndex].player2Bet;
                sprintf(mensajeGana, "\nHas ganado la mano, ganas %d monedas\n", games[gameIndex].player2Bet);
                copyGameStatusStructure(status, mensajeGana, &games[gameIndex].player1Deck, NULL);

            }else{ // Jugador 2
                
                games[gameIndex].player2Stack+= games[gameIndex].player1Bet;
                sprintf(mensajeGana, "\nHas ganado la mano, ganas %d monedas\n", games[gameIndex].player1Bet);
                copyGameStatusStructure(status, mensajeGana, &games[gameIndex].player2Deck, NULL);

            }
        }else if(ganadorMano==-1){ // Empate en la mano
            if (numJugador==player1){ // Jugador 1

                sprintf(mensajeEmpata, "\nEmpate, no pierdes monedas\n");
                copyGameStatusStructure(status, mensajeEmpata, &games[gameIndex].player1Deck, NULL);

            }else{ // Jugador 2

                sprintf(mensajeEmpata, "\nEmpate, no pierdes monedas\n");
                copyGameStatusStructure(status, mensajeEmpata, &games[gameIndex].player2Deck, NULL);
            }
        
        }else{ // Perdedor en la mano 
            if (numJugador==player1){ // Jugador 1

                games[gameIndex].player1Stack-= games[gameIndex].player1Bet;
                sprintf(mensajePierde, "\nHas perdido la mano, pierdes %d monedas\n", games[gameIndex].player1Bet);
                copyGameStatusStructure(status, mensajePierde, &games[gameIndex].player1Deck, NULL);
                
            }else{ // Jugador 2

                games[gameIndex].player2Stack-= games[gameIndex].player2Bet;
                sprintf(mensajePierde, "\nHas perdido la mano, pierdes %d monedas\n", games[gameIndex].player2Bet);
                copyGameStatusStructure(status, mensajePierde, &games[gameIndex].player2Deck, NULL);

            }
   
        }

        //comprobamos quien ha ganado la partida
        ganador= compruebaFin(&games[gameIndex].player1Stack, games[gameIndex].player2Stack);

        if(ganador!=-1){
            result=TRUE;
            games[gameIndex].endOfGame=TRUE;
            if (numJugador==ganador){ // Ganador de la partida
                if(numJugador=player1){ // Jugador 1

                    games[gameIndex].player1Stack+= games[gameIndex].player2Bet;
                    sprintf(mensajeGana, "\nHas ganado la partida");
                    copyGameStatusStructure(status, mensajeGana, &games[gameIndex].player1Deck, GAME_WIN);

                }else{ // Jugador 2
                    
                    games[gameIndex].player2Stack+= games[gameIndex].player1Bet;
                    sprintf(mensajeGana, "\nHas ganado la partida\n");
                    copyGameStatusStructure(status, mensajeGana, &games[gameIndex].player2Deck, GAME_WIN);

                }
            }else{ // Perdedor de la partida 
                if (numJugador==player1){ // Jugador 1

                    games[gameIndex].player1Stack-= games[gameIndex].player1Bet;
                    sprintf(mensajePierde, "\nHas perdido la partida\n", games[gameIndex].player1Bet);
                    copyGameStatusStructure(status, mensajePierde, &games[gameIndex].player1Deck, GAME_LOSE);
                    
                }else{ // Jugador 2

                    games[gameIndex].player2Stack-= games[gameIndex].player2Bet;
                    sprintf(mensajePierde, "\nHas perdido la partida\n", games[gameIndex].player2Bet);
                    copyGameStatusStructure(status, mensajePierde, &games[gameIndex].player2Deck, GAME_LOSE);

                }
            }
        }else{
            if(games[gameIndex].currentPlayer==numJugador){ // Turno del jugador
                if (numJugador==player1){ // Jugador 1
                    sprintf(mensaje, "\nEs tu turno, tienes %d puntos.\n", calculatePoints(&games[gameIndex].player1Deck));
                    copyGameStatusStructure(status, mensaje, &games[gameIndex].player1Deck, TURN_PLAY);
                }else{ // Jugador 2
                    sprintf(mensaje, "\nEs tu turno, tienes %d puntos.\n", calculatePoints(&games[gameIndex].player2Deck));
                    copyGameStatusStructure(status, mensaje, &games[gameIndex].player2Deck, TURN_PLAY);
                }
            }else{ // Turno del rival
                if (numJugador==player1){ // Jugador 1
                    sprintf(mensaje, "\nEspera tu turno\n");
                    copyGameStatusStructure(status, mensaje, &games[gameIndex].player1Deck, TURN_WAIT); 
                }else{ // Jugador 2
                    sprintf(mensaje, "\nEspera tu turno\n");
                    copyGameStatusStructure(status, mensaje, &games[gameIndex].player2Deck, TURN_WAIT); 
                }
            }
        }
    }
    return SOAP_OK;
}

int blackJackns__playermove(struct soap *soap, blackJackns__tMessage playerName, unsigned int playerMove, int* result){
    switch (playerMove)
    {
    case PLAYER_HIT_CARD:
        /* code */
        break;
    case PLAYER_STAND:
        /* code */
        break;
    default:
        break;
    }
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