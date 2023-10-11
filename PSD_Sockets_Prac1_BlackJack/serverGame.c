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

void pedirApuesta(unsigned int *playerStack, unsigned int *playerBet, int socketPlayer){
        int localTurnBet= TURN_BET;
        int localTurnBetOk= TURN_BET_OK;
        int apuestaCorrecta;

        send(socketPlayer, &localTurnBet, sizeof(localTurnBet), 0);
        send(socketPlayer, playerStack, 4, 0);
//      printf("&:%d\n", &playerStack);
//      printf("*:%d\n", *playerStack);
//      printf("_:%d\n", playerStack);

        recv(socketPlayer, playerBet, 4, 0);
//      printf("&:%d\n", &playerBet);
//      printf("*:%d\n",*playerBet);        
//      printf("_:%d\n",playerBet);        
        
        apuestaCorrecta=FALSE;
        while(!apuestaCorrecta){
                if(*playerBet <= *playerStack){ // Apuesta Correcta
                        apuestaCorrecta=TRUE;
                        send(socketPlayer, &localTurnBetOk, sizeof(localTurnBetOk), 0);
                }else{ // Apuesta incorrecta      
                        apuestaCorrecta=FALSE;
                        send(socketPlayer, &localTurnBet, sizeof(localTurnBet), 0);
                        recv(socketPlayer, playerBet, 4, 0);
                }
        }

}

void juega(int socketPlayer, tDeck *playerDeck, tDeck *gameDeck){
        unsigned int localTurnPlay= TURN_PLAY;
        unsigned int localTurnPlayOut= TURN_PLAY_OUT;
        unsigned int puntosJug=0;
        unsigned int pideCarta;
        unsigned int finalizoMano=FALSE;
        tString mensaje;
        send(socketPlayer, &localTurnPlay, sizeof(localTurnPlay), 0);
        puntosJug=calculatePoints(playerDeck);
        sprintf(mensaje, "Es tu turno, tienes %d puntos.\n", puntosJug);
        send(socketPlayer, &mensaje, sizeof(mensaje), 0);
        send(socketPlayer, playerDeck, sizeof(*playerDeck), 0);


        while (!finalizoMano){
                
                //      Recibimos si pide carta el jugador
                recv(socketPlayer, &pideCarta, sizeof(pideCarta), 0);
                if(pideCarta==TURN_PLAY_HIT){
                        playerDeck->cards[playerDeck->numCards]=getRandomCard(gameDeck);
                        playerDeck->numCards++;
                        puntosJug= calculatePoints(playerDeck);

                        if (puntosJug>GOAL_GAME){ // Se ha pasado
                                send(socketPlayer, &localTurnPlayOut, sizeof(localTurnPlayOut), 0);
                                sprintf(mensaje, "Tienes %d puntos, te has pasado.\n", puntosJug);
                                send(socketPlayer, &mensaje, sizeof(mensaje), 0);
                                send(socketPlayer, playerDeck, sizeof(playerDeck), 0);
                                finalizoMano=TRUE;
                        }else{ //puede seguir jugando
                                send(socketPlayer, &localTurnPlay, sizeof(localTurnPlay), 0);
                                sprintf(mensaje, "Puedes seguir jugando, tienes %d puntos.\n", puntosJug);
                                send(socketPlayer, &mensaje, sizeof(mensaje), 0);
                                send(socketPlayer, playerDeck, sizeof(playerDeck), 0);
                        }

                }else{
                        finalizoMano=TRUE;
                }
                
                printDeck(playerDeck);

        }
}

void espera(){

}

int compruebaFin(unsigned int *stackA, unsigned int *stackB){
        if (*stackA==0)
                return player1;
        else if (*stackB==0)
                return player2;
        else 
                return -1;
}

unsigned int compruebaGanadorMano(tDeck *deckP1, tDeck *deckP2){
        unsigned int puntosP1, puntosP2;
        puntosP1= calculatePoints(deckP1);
        puntosP2= calculatePoints(deckP2);
        if (puntosP1==puntosP2 || (puntosP1>GOAL_GAME && puntosP2>GOAL_GAME)) //empate
                return -1;
        else if(puntosP1>puntosP2 && puntosP1<=GOAL_GAME){
                return player1;
        }else if(puntosP2>puntosP1 && puntosP2<=GOAL_GAME){
                return player2;
        }else{
                return -1;
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
        tSession sesion;
        int bytesRead;
        int ganador;                            /** Player1->0, Player2->1, sigueJugando->2*/                             
        int ganadorMano;
        unsigned int puntosJug;
        unsigned int pideCarta;
        tString mensajeGana, mensajePierde, mensajeEmpata;

        ganador =-1;

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
        
        // Obtener longitud del ClientAddr
	clientLength = sizeof(player1Address);

	// Accept!
	socketPlayer1 = accept(socketfd, (struct sockaddr *) &player1Address, &clientLength);
        if (socketPlayer1 < 0)
		showError("ERROR en el Accept 1");	 
	
        socketPlayer2 = accept(socketfd, (struct sockaddr *) &player2Address, &clientLength);

        if (socketPlayer2 < 0)
		showError("ERROR en el Accept 2");
                
        initSession(&sesion);
        tPlayer starterPlayer= player1;


        recv(socketPlayer1, &bytesRead, sizeof(int), 0);
        recv(socketPlayer1, sesion.player1Name, bytesRead, 0);

        recv(socketPlayer2, &bytesRead, sizeof(int), 0);
        recv(socketPlayer2, sesion.player2Name, bytesRead, 0);


        unsigned int localTurnPlay= TURN_PLAY;
        unsigned int localTurnPlayWait= TURN_PLAY_WAIT;
        unsigned int localTurnPlayOut= TURN_PLAY_OUT;

        //Apuestas
        pedirApuesta(&sesion.player1Stack, &sesion.player1Bet, socketPlayer1);
        pedirApuesta(&sesion.player2Stack, &sesion.player2Bet, socketPlayer2);
        printSession(&sesion);

        while(ganador==-1){
                //Inicio de partida
                if(starterPlayer==player1){
                        juega(socketPlayer1, &sesion.player1Deck, &sesion.gameDeck);                
                        espera();
                        juega(socketPlayer1, &sesion.player1Deck, &sesion.gameDeck);
                        espera();
                }else{
                        juega(socketPlayer2, &sesion.player2Deck, &sesion.gameDeck);                
                        espera();
                        juega(socketPlayer1, &sesion.player1Deck, &sesion.gameDeck);                
                        espera();
                }
                clearDeck(&sesion.player1Deck);
                clearDeck(&sesion.player2Deck);
               
                ganadorMano= compruebaGanadorMano(&sesion.player1Deck, &sesion.player2Deck);
               
                unsigned int ganancias= sesion.player1Bet + sesion.player2Bet;

                if(ganadorMano==player1){

                        sesion.player1Stack+= sesion.player2Bet;
                        sesion.player2Stack-= sesion.player2Bet;
                        sprintf(mensajeGana, "Has ganado la mano, ganas %d monedas\n", sesion.player2Bet);
                        send(socketPlayer1, mensajeGana, sizeof(mensajeGana), 0);
                        sprintf(mensajePierde, "Has perdido la mano, pierdes %d monedas\n", sesion.player1Bet);
                        send(socketPlayer2, mensajePierde, sizeof(mensajePierde), 0);

                }else if(ganadorMano==player2){

                        sesion.player2Stack+= sesion.player1Bet;
                        sesion.player1Stack-= sesion.player1Bet;
                        sprintf(mensajeGana, "Has ganado la mano, ganas %d monedas\n", sesion.player1Bet);
                        send(socketPlayer2, mensajeGana, sizeof(mensajeGana), 0);
                        sprintf(mensajePierde, "Has perdido la mano, pierdes %d monedas\n", sesion.player2Bet);
                        send(socketPlayer1, mensajePierde, sizeof(mensajePierde), 0);

                }else{  //empate

                        sprintf(mensajeEmpata, "Empate, no pierdes monedas\n");
                        send(socketPlayer2, mensajeEmpata, sizeof(mensajeEmpata), 0);
                        send(socketPlayer1, mensajeEmpata, sizeof(mensajeEmpata), 0);

                }

                ganador= compruebaFin(&sesion.player1Stack, &sesion.player2Stack);
                if(ganador==player1){
                        sprintf(mensajeGana, "Has ganado la partida\n");
                        sprintf(mensajePierde, "Has perdido la partida\n");
                }else if(ganador==player2){
                        sprintf(mensajeGana, "Has ganado la partida\n");
                        sprintf(mensajePierde, "Has perdido la partida\n");
                }

                starterPlayer= getNextPlayer(starterPlayer);
        }
        /*
                //      TURN_PLAY JugA
                send(socketPlayer1, &localTurnPlay, sizeof(localTurnPlay), 0);
                puntosJug=calculatePoints(&sesion.player1Deck);
                sprintf(mensaje, "Es tu turno, tienes %d puntos.\n", puntosJug);
                send(socketPlayer1, &mensaje, sizeof(mensaje), 0);
                send(socketPlayer1, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);

                //      TURN_PLAY_WAIT JugB
                send(socketPlayer2, &localTurnPlayWait, sizeof(localTurnPlayWait), 0);
                send(socketPlayer2, &puntosJug, sizeof(puntosJug), 0);
                send(socketPlayer2, &sesion.player1Deck, sizeof(sesion.player1Deck), 0);

                //      Recibimos si pide carta el jugador
                recv(socketPlayer1, &pideCarta, sizeof(pideCarta), 0);

                while (pideCarta==TURN_PLAY_HIT){
                        
                        sesion.player1Deck.cards[sesion.player1Deck.numCards]=getRandomCard(&sesion.gameDeck);
                        sesion.player1Deck.numCards++;
                        puntosJug= calculatePoints(&sesion.player1Deck);
                        printDeck(&sesion.player1Deck);
                        

                        if (puntosJug>GOAL_GAME){ // Se ha pasado
                                send(socketPlayer1, &localTurnPlayOut, sizeof(localTurnPlayOut), 0);
                                pideCarta=TURN_PLAY_OUT;
                        }else{ //puede seguir jugando
                                send(socketPlayer1, &localTurnPlay, sizeof(localTurnPlay), 0);
                        }
                        
                        send(socketPlayer1, &puntosJug, sizeof(puntosJug), 0);
                        send(socketPlayer1, &(sesion.player1Deck), sizeof(sesion.player1Deck), 0);
                        recv(socketPlayer1, &pideCarta, sizeof(pideCarta), 0);
                }
        }

        */   
        return 0;
}
