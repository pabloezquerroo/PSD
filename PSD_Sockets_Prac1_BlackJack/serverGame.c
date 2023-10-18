#include "serverGame.h"
#include <pthread.h>

tPlayer getNextPlayer (tPlayer starterPlayer){

	tPlayer next;

		if (starterPlayer == player1)
			next = player2;
		else
			next = player1;

	return next;
}

void initDeck (tDeck *deck){

	deck->numCards = DECK_SIZE;

	for (int i=0; i<DECK_SIZE; i++){
		deck->cards[i] = i;
	}
}

void clearDeck (tDeck *deck){

	// Set number of cards
	deck->numCards = 0;

	for (int i=0; i<DECK_SIZE; i++){
		deck->cards[i] = UNSET_CARD;
	}
}

void printSession (tSession *session){

		printf ("\n ------ Session state ------\n");

		// Player 1
		printf ("%s [bet:%d; %d chips] Deck:", session->player1Name, session->player1Bet, session->player1Stack);
		printDeck (&(session->player1Deck));

		// Player 2
		printf ("%s [bet:%d; %d chips] Deck:", session->player2Name, session->player2Bet, session->player2Stack);
		printDeck (&(session->player2Deck));

		// Current game deck
		if (DEBUG_PRINT_GAMEDECK){
			printf ("Game deck: ");
			printDeck (&(session->gameDeck));
		}
}

void initSession (tSession *session){

	clearDeck (&(session->player1Deck));
	session->player1Bet = 0;
	session->player1Stack = INITIAL_STACK;

	clearDeck (&(session->player2Deck));
	session->player2Bet = 0;
	session->player2Stack = INITIAL_STACK;

	initDeck (&(session->gameDeck));
}

unsigned int calculatePoints (tDeck *deck){

	unsigned int points;

		// Init...
		points = 0;

		for (int i=0; i<deck->numCards; i++){

			if (deck->cards[i] % SUIT_SIZE < 9)
				points += (deck->cards[i] % SUIT_SIZE) + 1;
			else
				points += FIGURE_VALUE;
		}

	return points;
}

unsigned int getRandomCard (tDeck* deck){

	unsigned int card, cardIndex, i;

		// Get a random card
		cardIndex = rand() % deck->numCards;
		card = deck->cards[cardIndex];

		// Remove the gap
		for (i=cardIndex; i<deck->numCards-1; i++)
			deck->cards[i] = deck->cards[i+1];

		// Update the number of cards in the deck
		deck->numCards--;
		deck->cards[deck->numCards] = UNSET_CARD;

	return card;
}


void pedirApuesta(unsigned int *playerStack, unsigned int *playerBet, int socketPlayer, tDeck *playerDeck){
        int localTurnBet= TURN_BET;
        int localTurnBetOk= TURN_BET_OK;
        int apuestaCorrecta;
        int tamMensaje;
        tString mensajeApostar;

        send(socketPlayer, &localTurnBet, sizeof(localTurnBet), 0);
        sprintf(mensajeApostar, "\nTurno de apuesta. %d fichas disponibles\n", *playerStack);
        tamMensaje=sizeof(mensajeApostar);
        send(socketPlayer, &tamMensaje, 4, 0);
        send(socketPlayer, &mensajeApostar, tamMensaje, 0);
        send(socketPlayer, playerDeck, sizeof(*playerDeck), 0);

        recv(socketPlayer, playerBet, 4, 0);
        apuestaCorrecta=FALSE;
        while(!apuestaCorrecta){
                if(*playerBet <= *playerStack && *playerBet>0 && *playerBet<=MAX_BET){ // Apuesta Correcta
                        apuestaCorrecta=TRUE;
                        send(socketPlayer, &localTurnBetOk, sizeof(localTurnBetOk), 0);
                        sprintf(mensajeApostar, "Apuestas %d fichas\n", *playerBet);
                        tamMensaje=sizeof(mensajeApostar);
                        send(socketPlayer, &tamMensaje, 4, 0);
                        send(socketPlayer, &mensajeApostar, tamMensaje, 0);
                        send(socketPlayer, playerDeck, sizeof(*playerDeck), 0);
                }else{ // Apuesta incorrecta      
                        apuestaCorrecta=FALSE;
                        send(socketPlayer, &localTurnBet, sizeof(localTurnBet), 0);
                        sprintf(mensajeApostar, "Apuestas incorrecta");
                        tamMensaje=sizeof(mensajeApostar);
                        send(socketPlayer, &tamMensaje, 4, 0);
                        send(socketPlayer, &mensajeApostar, tamMensaje, 0);
                        send(socketPlayer, playerDeck, sizeof(*playerDeck), 0);
                        recv(socketPlayer, playerBet, 4, 0);
                } 
        }

}

void juega(int socketPlayerAct, int socketPlayerPas, tDeck *playerDeckAct, tDeck *gameDeck){
        unsigned int localTurnPlay= TURN_PLAY;
        unsigned int localTurnPlayOut= TURN_PLAY_OUT;
        unsigned int localTurnPlayWait= TURN_PLAY_WAIT;
        unsigned int puntosJug=0;
        unsigned int pideCarta;
        unsigned int finalizoMano=FALSE;
        int tamMensaje;
        tString mensaje;
        
        //Activo
        send(socketPlayerAct, &localTurnPlay, sizeof(localTurnPlay), 0);
        puntosJug=calculatePoints(playerDeckAct);
        sprintf(mensaje, "\nEs tu turno, tienes %d puntos.", puntosJug);
        tamMensaje=sizeof(mensaje);
        send(socketPlayerAct, &tamMensaje, 4, 0);
        send(socketPlayerAct, &mensaje, tamMensaje, 0);
        send(socketPlayerAct, playerDeckAct, sizeof(*playerDeckAct), 0);

        while (!finalizoMano){
                
                //      Recibimos si pide carta el jugador
                recv(socketPlayerAct, &pideCarta, sizeof(pideCarta), 0);
                if(pideCarta==TURN_PLAY_HIT){
                        playerDeckAct->cards[playerDeckAct->numCards]=getRandomCard(gameDeck);
                        (playerDeckAct->numCards)++;
                        puntosJug= calculatePoints(playerDeckAct);
                        
                        if (puntosJug>GOAL_GAME){ // Se ha pasado
                                send(socketPlayerAct, &localTurnPlayOut, sizeof(localTurnPlayOut), 0);
                                sprintf(mensaje, "\nTienes %d puntos, te has pasado.\n", puntosJug);
                                send(socketPlayerAct, &tamMensaje, 4, 0);
                                send(socketPlayerAct, &mensaje, tamMensaje, 0);
                                send(socketPlayerAct, playerDeckAct, sizeof(*playerDeckAct), 0);
                                finalizoMano=TRUE;
                        }else{ //puede seguir jugando
                                send(socketPlayerAct, &localTurnPlay, sizeof(localTurnPlay), 0);
                                sprintf(mensaje, "Puedes seguir jugando, tienes %d puntos.", puntosJug);
                                send(socketPlayerAct, &tamMensaje, 4, 0);
                                send(socketPlayerAct, &mensaje, tamMensaje, 0);
                                send(socketPlayerAct, playerDeckAct, sizeof(*playerDeckAct), 0);
                        }

                }else{
                        send(socketPlayerAct, &localTurnPlayWait, sizeof(localTurnPlayOut), 0);
                        sprintf(mensaje, "Te plantas con %d puntos.", puntosJug);
                        send(socketPlayerAct, &tamMensaje, 4, 0);
                        send(socketPlayerAct, &mensaje, tamMensaje, 0);
                        send(socketPlayerAct, playerDeckAct, sizeof(*playerDeckAct), 0);
                        finalizoMano=TRUE;
                }
                      
                //Pasivo
                send(socketPlayerPas, &localTurnPlayWait, sizeof(localTurnPlay), 0);
                sprintf(mensaje, "\nTu rival tiene %d puntos.", puntosJug);
                tamMensaje=sizeof(mensaje);
                send(socketPlayerPas, &tamMensaje, 4, 0);
                send(socketPlayerPas, &mensaje, tamMensaje, 0);
                send(socketPlayerPas, playerDeckAct, sizeof(*playerDeckAct), 0);

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

unsigned int compruebaGanadorMano(tDeck *deckP1, tDeck *deckP2){
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

void partida(tThreadArgs *threadArgs){
        unsigned int localTurnPlayRivalDone= TURN_PLAY_RIVAL_DONE;
        unsigned int localTurnGameWin=TURN_GAME_WIN;
        unsigned int localTurnGameLose=TURN_GAME_LOSE;
        
        tSession sesion;
        int ganador;                            /** Player1->0, Player2->1, sigueJugando->-1*/                             
        int bytesRead;
        int tamMensaje;
        int ganadorMano;
        tString mensajeGana, mensajePierde, mensajeEmpata;
        //ARGS
        int socketPlayer1= threadArgs->socketPlayer1;	/** Socket descriptor for player 1 */
	int socketPlayer2= threadArgs->socketPlayer2;	/** Socket descriptor for player 2 */

        ganador =-1;
        initSession(&sesion);
        tPlayer starterPlayer= player1;


        recv(socketPlayer1, &bytesRead, sizeof(int), 0);
        recv(socketPlayer1, sesion.player1Name, bytesRead, 0);

        recv(socketPlayer2, &bytesRead, sizeof(int), 0);
        recv(socketPlayer2, sesion.player2Name, bytesRead, 0);

        printf("llega\n");

        while(ganador==-1){
                system("clear");
                //Apuestas
                pedirApuesta(&sesion.player1Stack, &sesion.player1Bet, socketPlayer1, &sesion.player1Deck);
                pedirApuesta(&sesion.player2Stack, &sesion.player2Bet, socketPlayer2, &sesion.player2Deck);
                printSession(&sesion);
                
                //Inicio de partida
                if(starterPlayer==player1){
                        juega(socketPlayer1, socketPlayer2, &sesion.player1Deck, &sesion.gameDeck); 
                        juega(socketPlayer2, socketPlayer1, &sesion.player2Deck, &sesion.gameDeck);
                        send(socketPlayer1, &localTurnPlayRivalDone, sizeof(localTurnPlayRivalDone), 0);
                        send(socketPlayer2, &localTurnPlayRivalDone, sizeof(localTurnPlayRivalDone), 0);

                }else{
                        juega(socketPlayer2, socketPlayer1, &sesion.player2Deck, &sesion.gameDeck);                
                        juega(socketPlayer1, socketPlayer2, &sesion.player1Deck, &sesion.gameDeck);         
                        send(socketPlayer2, &localTurnPlayRivalDone, sizeof(localTurnPlayRivalDone), 0);
                        send(socketPlayer1, &localTurnPlayRivalDone, sizeof(localTurnPlayRivalDone), 0);
                }
                ganadorMano= compruebaGanadorMano(&sesion.player1Deck, &sesion.player2Deck);
               
                if(ganadorMano==player1){

                        sesion.player1Stack+= sesion.player2Bet;
                        sesion.player2Stack-= sesion.player2Bet;
                        sprintf(mensajeGana, "\nHas ganado la mano, ganas %d monedas", sesion.player2Bet);
                        tamMensaje=sizeof(mensajeGana);
                        send(socketPlayer1, &tamMensaje, 4, 0);
                        send(socketPlayer1, &mensajeGana, tamMensaje, 0);
                        send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);

                        sprintf(mensajePierde, "\nHas perdido la mano, pierdes %d monedas", sesion.player2Bet);
                        tamMensaje=sizeof(mensajePierde);
                        send(socketPlayer2, &tamMensaje, 4, 0);
                        send(socketPlayer2, &mensajePierde, tamMensaje, 0);
                        send(socketPlayer2, &sesion.player2Deck, sizeof(sesion.player2Deck), 0);

                }else if(ganadorMano==player2){

                        sesion.player2Stack+= sesion.player1Bet;
                        sesion.player1Stack-= sesion.player1Bet;
                        sprintf(mensajeGana, "\nHas ganado la mano, ganas %d monedas", sesion.player1Bet);
                        tamMensaje=sizeof(mensajeGana);                        
                        send(socketPlayer2, &tamMensaje, 4, 0);
                        send(socketPlayer2, &mensajeGana, tamMensaje, 0);
                        send(socketPlayer2, &sesion.player2Deck, sizeof(sesion.player2Deck), 0);

                        sprintf(mensajePierde, "\nHas perdido la mano, pierdes %d monedas", sesion.player1Bet);
                        tamMensaje=sizeof(mensajePierde);
                        send(socketPlayer1, &tamMensaje, 4, 0);
                        send(socketPlayer1, &mensajePierde, tamMensaje, 0);
                        send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);


                }else{  //empate

                        sprintf(mensajeEmpata, "\nEmpate, no pierdes monedas\n");
                        tamMensaje=sizeof(mensajeEmpata);

                        send(socketPlayer2, &tamMensaje, 4, 0);
                        send(socketPlayer2, &mensajeEmpata, tamMensaje, 0);
                        send(socketPlayer2, &sesion.player2Deck, sizeof(sesion.player2Deck), 0);

                        send(socketPlayer1, &tamMensaje, 4, 0);
                        send(socketPlayer1, &mensajeEmpata, tamMensaje, 0);
                        send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);

                }

                ganador= compruebaFin(&sesion.player1Stack, &sesion.player2Stack);
                if(ganador==player1){
                        
                        send(socketPlayer1, &localTurnGameWin, sizeof(localTurnGameWin), 0);
                        sprintf(mensajeGana, "Has ganado la partida\n");
                        tamMensaje=sizeof(mensajeGana);
                        send(socketPlayer1, &tamMensaje, 4, 0);
                        send(socketPlayer1, &mensajeGana, tamMensaje, 0);
                        send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);

                        send(socketPlayer2, &localTurnGameLose, sizeof(localTurnGameLose), 0);
                        sprintf(mensajePierde, "Has perdido la partida\n");
                        tamMensaje=sizeof(mensajePierde);
                        send(socketPlayer2, &tamMensaje, 4, 0);
                        send(socketPlayer2, &mensajePierde, tamMensaje, 0);
                        send(socketPlayer2, &sesion.player2Deck, sizeof(sesion.player2Deck), 0);
                }else if(ganador==player2){

                        send(socketPlayer2, &localTurnGameWin, sizeof(localTurnGameWin), 0);
                        sprintf(mensajeGana, "Has ganado la partida\n");
                        tamMensaje=sizeof(mensajeGana);
                        send(socketPlayer2, &tamMensaje, 4, 0);
                        send(socketPlayer2, &mensajeGana, tamMensaje, 0);
                        send(socketPlayer2, &sesion.player2Deck, sizeof(sesion.player2Deck), 0);

                        send(socketPlayer1, &localTurnGameLose, sizeof(localTurnGameLose), 0);
                        sprintf(mensajePierde, "Has perdido la partida\n");
                        tamMensaje=sizeof(mensajePierde);
                        send(socketPlayer1, &tamMensaje, 4, 0);
                        send(socketPlayer1, &mensajePierde, tamMensaje, 0);
                        send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);
                }

                clearDeck(&sesion.player1Deck);
                clearDeck(&sesion.player2Deck);

                starterPlayer= getNextPlayer(starterPlayer);
        }

}

int main(int argc, char *argv[]){

	int socketfd;				/** Socket descriptor */
	struct sockaddr_in serverAddress;	/** Server address structure */
	unsigned int port;			/** Listening port */
	struct sockaddr_in player1Address;	/** Client address structure for player 1 */
	struct sockaddr_in player2Address;	/** Client address structure for player 2 */
	int socketPlayer1;			/** Socket descriptor for player 1 */
	int socketPlayer2;			/** Socket descriptor for player 2 */
	unsigned int clientLength;		/** Length of client structure */
	tThreadArgs *threadArgs; 		/** Thread parameters */
	pthread_t threadID;			/** Thread ID */

        // Seed
        srand(time(0));

        // Check arguments
        if (argc != 2) {
                fprintf(stderr,"ERROR wrong number of arguments\n");
                fprintf(stderr,"Usage:\n$>%s port\n", argv[0]);
                exit(1);
        }

        socketfd= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // Check
        if (socketfd < 0)
                showError("ERROR al abrir el puerto");

        // Inicializar struct servidor
        memset(&serverAddress, 0, sizeof(serverAddress));

        // Obtener listening port
        port = atoi(argv[1]);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);
    
	if (bind(socketfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
                perror("Error al asociar un puerto\n");

       	// Listen 
        listen(socketfd, 10);


        while(1){
                // Accept!
                clientLength = sizeof(player1Address);
                socketPlayer1 = accept(socketfd, (struct sockaddr *) &player1Address, &clientLength);
                if (socketPlayer1 < 0)
                        showError("ERROR en el Accept 1");	 

                socketPlayer2 = accept(socketfd, (struct sockaddr *) &player2Address, &clientLength);
                if (socketPlayer2 < 0)
                        showError("ERROR en el Accept 2");

                threadArgs= malloc(sizeof(tThreadArgs));
                threadArgs->socketPlayer1= socketPlayer1;
                threadArgs->socketPlayer2= socketPlayer2;

                if(pthread_create(&threadID, NULL, partida, threadArgs)!=0)
                        showError("ERROR en el la creacion del hilo");
        }
        close(socketfd);

        return 0;
}
