#include "soapH.h"
#include "wsConcat.nsmap"

int main(int argc, char **argv){

  struct soap soap;
  wsConcatns__tMessage myMsgA, myMsgB, myMsgC;
  char *serverURL;

	// Usage
  	if (argc != 4) {
	   printf("Usage: %s http://server:port messageA messageB\n",argv[0]);
	   exit(0);
  	}

	// Init gSOAP environment
  	soap_init(&soap);
  	
	// Obtain server address
	serverURL = argv[1];			

	//Init the messages from client-side
	
		// Alloc memory, init to zero and copy the message
		myMsgA.__ptr = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
		bzero (myMsgA.__ptr, IMS_MAX_MSG_SIZE);
		strcpy (myMsgA.__ptr, argv[2]);
		myMsgA.__size = strlen (argv[2])+1;

		// Alloc memory, init to zero and copy the name
		myMsgB.__ptr = (xsd__string) malloc (IMS_MAX_MSG_SIZE);
		bzero (myMsgB.__ptr, IMS_MAX_MSG_SIZE);
		strcpy (myMsgB.__ptr, argv[3]);
		myMsgB.__size = strlen (argv[3]) + 1;

		printf ("Sending message A:%s (%d chars)\n", myMsgA.__ptr, myMsgA.__size);
		printf ("Sending message B:%s (%d chars)\n", myMsgB.__ptr, myMsgB.__size);

		// Send this message to the server
    	soap_call_wsConcatns__remoteConcat (&soap, serverURL, "", myMsgA, myMsgB, &myMsgC);
	
 		// Check for errors...
  		if (soap.error) {
      		soap_print_fault(&soap, stderr); 
			exit(1);
  		}
		else
			printf ("Received: %s (%d chars)\n", myMsgC.__ptr, myMsgC.__size);    
  	
	
	// Clean the environment
	soap_destroy(&soap);
  	soap_end(&soap);
  	soap_done(&soap);

  return 0;
}
