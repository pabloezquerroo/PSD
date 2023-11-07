#include "client.h"

unsigned int readBet (){

	int isValid, bet=0;
	xsd__string enteredMove;

		// While player does not enter a correct bet...
		do{

			// Init...
			enteredMove = (xsd__string) malloc (STRING_LENGTH);
			bzero (enteredMove, STRING_LENGTH);
			isValid = TRUE;

			printf ("Enter a value:");
			fgets(enteredMove, STRING_LENGTH-1, stdin);
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (int i=0; i<strlen(enteredMove) && isValid; i++)
				if (!isdigit(enteredMove[i]))
					isValid = FALSE;

			// Entered move is not a number
			if (!isValid)
				printf ("Entered value is not correct. It must be a number greater than 0\n");
			else
				bet = atoi (enteredMove);

		}while (!isValid);

		printf ("\n");
		free (enteredMove);

	return ((unsigned int) bet);
}

unsigned int readOption (){

	unsigned int bet;

		do{
			printf ("What is your move? Press %d to hit a card and %d to stand\n", PLAYER_HIT_CARD, PLAYER_STAND);
			bet = readBet();
			if ((bet != PLAYER_HIT_CARD) && (bet != PLAYER_STAND))
				printf ("Wrong option!\n");			
		} while ((bet != PLAYER_HIT_CARD) && (bet != PLAYER_STAND));

	return bet;
}

int main(int argc, char **argv){

	struct soap soap;					/** Soap struct */
	char *serverURL;					/** Server URL */
	blackJackns__tMessage playerName;	/** Player name */
	blackJackns__tBlock gameStatus;		/** Game status */
	unsigned int playerMove;			/** Player's move */
	int resCode, gameId;				/** Result and gameId */
	unsigned int endOfGame;
		// Init gSOAP environment
		soap_init(&soap);

		// Obtain server address
		serverURL = argv[1];

		// Allocate memory
		allocClearMessage (&soap, &(playerName));
		allocClearBlock (&soap, &gameStatus);
				
		// Check arguments
		if (argc !=2) {
			printf("Usage: %s http://server:port\n",argv[0]);
			exit(0);
		}

		endOfGame=FALSE;

		// Init and read the message

		resCode=-1;
		while(resCode <= 0){ // Mientras no registrado
			printf("Enter your name: ");
			memset(gameStatus.msgStruct.msg, 0, STRING_LENGTH);
			fgets(playerName.msg, STRING_LENGTH-1, stdin);
			playerName.__size= strlen(playerName.msg)-1;
			soap_call_blackJackns__register(&soap, serverURL, "", playerName, &resCode);
			switch (resCode)
			{
			case ERROR_NAME_REPEATED:
					printf("Error: Nombre ya existente\n");
				break;
			case ERROR_SERVER_FULL:
					printf("Error: Servidor completo\n");
				break;
			default:
					printf("¡Registro Correcto! Partida %d\n", resCode);
					gameId=resCode;
				break;
			}
		}
		while(!endOfGame){
			soap_call_blackJackns__getStatus(&soap, serverURL, "", playerName, &gameStatus, &resCode);
			printf("Game status: %s\n", gameStatus.msgStruct.msg);
			printf("Mi mazo: \n");
			printFancyDeck(&gameStatus.deck.cards);
			if (resCode==TRUE){ // partida finalizada
				endOfGame=TRUE;
			}else{
				while (gameStatus.code==TURN_PLAY){
					playerMove=readOption();
					soap_call_blackJackns__playermove(&soap, serverURL, "", playerName, playerMove, &gameStatus);
					printf(gameStatus.msgStruct.msg);
					printf("Mi mazo: \n");
					printFancyDeck(&gameStatus.deck.cards);
				}
			}
		}

  	return 0;
}