
#include "soapH.h"
#include "wsConcat.nsmap"

int main(int argc, char **argv){ 

  int primarySocket, secondarySocket;
  struct soap soap;

  	if (argc < 2) {
    	printf("Usage: %s <port>\n",argv[0]); 
		exit(-1); 
  	}

	// Init environment
  	soap_init(&soap);

	// Bind to the specified port	
	primarySocket = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding		
	if (primarySocket < 0) {
  		soap_print_fault(&soap, stderr); 
		exit(-1); 
	}

	// Listen to next connection
	while (1) { 

		// accept
	  	secondarySocket = soap_accept(&soap);    

	  	if (secondarySocket < 0) {
			soap_print_fault(&soap, stderr); exit(-1);
	  	}

		// Execute invoked operation
	  	soap_serve(&soap);

		// Clean up!
	  	soap_end(&soap);
	}

  return 0;
}


int wsConcatns__remoteConcat (struct soap *soap, wsConcatns__tMessage msgA, wsConcatns__tMessage msgB, wsConcatns__tMessage* msgC){

	xsd__string auxPtr;

		// Print without marking end-of-string
		printf ("Message A: %s (%d chars)\n", msgA.__ptr, msgA.__size);
		printf ("Message B: %s (%d chars)\n", msgB.__ptr, msgB.__size);	

		// Alloc memory for response message
		msgC->__ptr = (xsd__string) malloc (2*IMS_MAX_MSG_SIZE);
		memset (msgC->__ptr, 0, 2*IMS_MAX_MSG_SIZE);
		auxPtr = msgC->__ptr;

		// Copy first string
		strcpy (auxPtr, msgA.__ptr);
		auxPtr += strlen (msgA.__ptr);
		strcpy (auxPtr, msgB.__ptr);
		msgC->__size = msgA.__size + msgB.__size+1;

		printf ("Message C: %s (%d chars)\n", msgC->__ptr, msgC->__size);

	return SOAP_OK;
}


