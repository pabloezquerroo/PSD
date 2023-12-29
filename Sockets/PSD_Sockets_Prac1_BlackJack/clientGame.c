#include "clientGame.h"

void showCode (unsigned int code){

	tString string;

		if (DEBUG_CLIENT){

			// Reset
			memset (string, 0, STRING_LENGTH);

			switch(code){

				case TURN_BET:
					strcpy (string, "TURN_BET");
					break;

				case TURN_BET_OK:
					strcpy (string, "TURN_BET_OK");
					break;

				case TURN_PLAY:
					strcpy (string, "TURN_PLAY");
					break;

				case TURN_PLAY_HIT:
					strcpy (string, "TURN_PLAY_HIT");
					break;

				case TURN_PLAY_STAND:
					strcpy (string, "TURN_PLAY_STAND");
					break;

				case TURN_PLAY_OUT:
					strcpy (string, "TURN_PLAY_OUT");
					break;

				case TURN_PLAY_WAIT:
					strcpy (string, "TURN_PLAY_WAIT");
					break;

				case TURN_PLAY_RIVAL_DONE:
					strcpy (string, "TURN_PLAY_RIVAL_DONE");
					break;

				case TURN_GAME_WIN:
					strcpy (string, "TURN_GAME_WIN");
					break;

				case TURN_GAME_LOSE:
					strcpy (string, "TURN_GAME_LOSE");
					break;

				default:
					strcpy (string, "UNKNOWN CODE");
					break;
			}

			printf ("Received:%s\n", string);
		}
}

unsigned int readBet (){

	int isValid, bet=0;
	tString enteredMove;

		// While player does not enter a correct bet...
		do{

			// Init...
			bzero (enteredMove, STRING_LENGTH);
			isValid = TRUE;

			printf ("Enter a bet:");
			fgets(enteredMove, STRING_LENGTH-1, stdin);
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (int i=0; i<strlen(enteredMove) && isValid; i++)
				if (!isdigit(enteredMove[i]))
					isValid = FALSE;

			// Entered move is not a number
			if (!isValid)
				printf ("Entered bet is not correct. It must be a number greater than 0\n");
			else
				bet = atoi (enteredMove);

		}while (!isValid);

		printf ("\n");

	return ((unsigned int) bet);
}

unsigned int readOption (){

	int isValid, option = 0;
	tString enteredMove;

		// While player does not enter a correct bet...
		do{
			// Init...
			bzero (enteredMove, STRING_LENGTH);
			isValid = TRUE;

			printf ("\nPress %d to hit a card and %d to stand:", PLAYER_HIT_CARD, PLAYER_STAND);
			fgets(enteredMove, STRING_LENGTH-1, stdin);
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (int i=0; i<strlen(enteredMove) && isValid; i++)
				if (!isdigit(enteredMove[i]))
					isValid = FALSE;

			if (!isValid)
				printf ("Wrong option!\n");
			else{

				// Conver entered text to an int
				option = atoi (enteredMove);

				if ((option != PLAYER_HIT_CARD) && (option != PLAYER_STAND)){
					printf ("Wrong option!\n");
					isValid = FALSE;
				}
			}
		}while (!isValid);

		printf ("\n");

	return ((unsigned int) option);
}


void tocaJugar(int socketfd){
        unsigned int localTurnPlayHit=TURN_PLAY_HIT;
        unsigned int localTurnPlayStand=TURN_PLAY_STAND;
        unsigned int opcion = readOption();
        if(opcion == PLAYER_STAND){   // Se planta
                send(socketfd, &localTurnPlayStand, sizeof(localTurnPlayStand), 0);
        }else{  // Pide Carta
                send(socketfd, &localTurnPlayHit, sizeof(localTurnPlayHit), 0);
        }
}

void tocaApostar(int socketfd){
        unsigned int apuesta;                                   /** Apuesta realizada*/
        apuesta=readBet();
        send(socketfd, &apuesta, sizeof(apuesta), 0);
}



int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	unsigned int port;					/** Port number (server) */
	struct sockaddr_in server_address;	/** Server address structure */
	char* serverIP;						/** Server IP */
	unsigned int endOfGame;				/** Flag to control the end of the game */
	tString playerName;					/** Name of the player */
	int nameLength;
	unsigned int turno;                                     /** Codigo par saber si me toca jugar*/
	int tamMensaje;
	tString mensaje;                                        /** mensaje desde servidor*/
	tDeck deck;                                             /** deck jugador*/


	// Check arguments!
	if (argc != 3){
		fprintf(stderr,"ERROR wrong number of arguments\n");
		fprintf(stderr,"Usage:\n$>%s serverIP port\n", argv[0]);
		exit(0);
	}
			endOfGame=0;

	// Get the server address
	serverIP = argv[1];

	// Get the port
	port = atoi(argv[2]);

			//Create Socket
			socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			// Check if the socket has been successfully created
	if (socketfd < 0)
		showError("ERROR while creating the socket");

			// Fill server address structure
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(serverIP);
	server_address.sin_port = htons(port);

			// Connect with server
	if (connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		showError("ERROR while establishing connection");

			// Init and read the message
			system("clear");
	printf("Enter your name: ");
	memset(playerName, 0, STRING_LENGTH);
	fgets(playerName, STRING_LENGTH-1, stdin);

	// Send message to the server side
	unsigned int tamNombre= sizeof(playerName);
	send(socketfd, &tamNombre , sizeof(int), 0);
	nameLength = send(socketfd, &playerName, strlen(playerName), 0);

	// Check the number of bytes sent
	if (nameLength < 0)
		showError("ERROR while writing to the socket");


	while(!endOfGame){
		recv(socketfd, &turno, sizeof(turno), 0);
		recv(socketfd, &tamMensaje, 4, 0);
		recv(socketfd, &mensaje, tamMensaje, 0);
		recv(socketfd, &deck, sizeof(deck), 0);    
		printf("%s\n",mensaje);
		if(turno==TURN_BET){
				tocaApostar(socketfd);
				system("clear");
		}else if(turno==TURN_PLAY){             // juegas
				printFancyDeck(&deck);
				tocaJugar(socketfd);
		}else if(turno==TURN_GAME_WIN || turno==TURN_GAME_LOSE){
				endOfGame=TRUE;
		}else if(turno==TURN_PLAY_WAIT){
				printf("Deck rival: ");
				printFancyDeck(&deck);
		}           
	}

	close(socketfd);
		
}

