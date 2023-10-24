#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_LENGTH 256		// Maximum length of the message

/** Function that shows an error message */
void showError(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){

     int socketfd;						/** Socket descriptor */
     unsigned short portServer;			/** Listening port */
     struct sockaddr_in serverAddress;	/** Server address structure */
     struct sockaddr_in clientAddress;	/** Client address structure */
	 socklen_t clientLength;			/** Length of client structure */
     char message[MAX_MSG_LENGTH];		/** Message */
     int messageLength;					/** Length of the message */


     	 // Check arguments
		 if (argc < 2) {
			 fprintf(stderr,"ERROR wrong number of arguments\n");
			 fprintf(stderr,"Usage:\n$>%s port\n", argv[0]);
			 exit(1);
		 }

		 // Create the socket
		 socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		 // Check
		 if (socketfd < 0)
			showError("ERROR while opening socket");
			
		 // Get listening port
		 portServer = atoi(argv[1]);

		 // Init server structure
		 memset(&serverAddress, 0, sizeof(serverAddress));	 

		 // Fill server structure
		 serverAddress.sin_family = AF_INET;
		 serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		 serverAddress.sin_port = htons(portServer);

		 // Bind
		 if (bind(socketfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
			 showError("ERROR while binding");		 

		 // Get length of client structure
		 clientLength = sizeof(clientAddress);

		 // Init message
		 memset(message, 0, MAX_MSG_LENGTH);		 
		 
		 // Receive message
		 messageLength = recvfrom(socketfd, 
		 							message, 
		 							MAX_MSG_LENGTH-1, 
		 							0,
		 							(struct sockaddr *) &clientAddress, 
		 							&clientLength);

		 // Check recieved bytes
		 if (messageLength < 0)
			 showError("ERROR while reading from socket");

		 // Show message
		 printf("Message: %s\n", message);

		 // Get the message length
		 memset (message, 0, MAX_MSG_LENGTH);
		 strcpy (message, "Message received!");
		 
		 // Send response message
		 messageLength = sendto(socketfd, 
		 						message, 
		 						strlen(message), 
		 						0,
		 						(struct sockaddr *) &clientAddress, 
		 						sizeof(clientAddress));

		 // Check bytes sent
		 if (messageLength < 0)
			 showError("ERROR while writing to socket");

		 // Close sockets
		 close(socketfd);

     return 0; 
}


