#include "serverGame.h"
#include <pthread.h>

tPlayer getNextPlayer (tPlayer currentPlayer){

	tPlayer next;

		if (currentPlayer == player1)
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
//      printf("_:%d\n", *playerStack);

        recv(socketPlayer, playerBet, 4, 0);
//      printf("&:%d\n", &playerBet);
//      printf("_:%d\n",*playerBet);        
        
        apuestaCorrecta=FALSE;
        while(!apuestaCorrecta){
                if(*playerBet < *playerStack){ // Apuesta Correcta
                        apuestaCorrecta=TRUE;
                        send(socketPlayer, &localTurnBetOk, sizeof(localTurnBetOk), 0);
                }else{ // Apuesta incorrecta      
                        apuestaCorrecta=FALSE;
                        send(socketPlayer, &localTurnBet, sizeof(localTurnBet), 0);
                        recv(socketPlayer, playerBet, 4, 0);
                        
                }
        }
}


int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	struct sockaddr_in serverAddress;	/** Server address structure */
	unsigned int port;					/** Listening port */
	struct sockaddr_in player1Address;	/** Client address structure for player 1 */
	struct sockaddr_in player2Address;	/** Client address structure for player 2 */
	int socketPlayer1;					/** Socket descriptor for player 1 */
	int socketPlayer2;					/** Socket descriptor for player 2 */
	unsigned int clientLength;			/** Length of client structure */
	tThreadArgs *threadArgs; 			/** Thread parameters */
	pthread_t threadID;					/** Thread ID */
        tSession sesion;
        int bytesRead;
        int end=0;

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

        recv(socketPlayer1, bytesRead, sizeof(int), 0);
        bytesRead=recv(socketPlayer1, sesion.player1Name, bytesRead, 0);
        if(bytesRead<0)
                showError("ERROR lectura de nombre 1");	 

        recv(socketPlayer2, bytesRead, sizeof(int), 0);
        bytesRead=recv(socketPlayer2, sesion.player2Name, bytesRead, 0);
        if(bytesRead<0)
                showError("ERROR lectura de nombre 2");	 

	printSession(&sesion);

        while(!end){
                pedirApuesta(&sesion.player1Stack, &sesion.player1Bet, socketPlayer1);
                pedirApuesta(&sesion.player2Stack, &sesion.player2Bet, socketPlayer2);
                printf("Apuesta player 1: %d\n", sesion.player1Bet);
                printf("Apuesta player 2: %d\n", sesion.player2Bet);
                end=TRUE;
        }

        return 0;
}
