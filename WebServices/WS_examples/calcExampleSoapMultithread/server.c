#include <pthread.h>
#include "soapH.h"
#include "calculator.nsmap"

#define TRUE 1
#define FALSE 0

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

		// Init soap environment
		soap_init(&soap);

		// Configure timeouts
		soap.send_timeout = 60; // 60 seconds
		soap.recv_timeout = 60; // 60 seconds
		soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
		soap.max_keep_alive = 100; // max keep-alive sequence

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


int calcns__add (struct soap *soap, int a, int b, int *res){  
  printf ("Executing (+)...\n");
  sleep (5);
  *res = a + b;
  return SOAP_OK;
}

int calcns__subs (struct soap *soap, int a, int b, int *res){  
  printf ("Executing (-)...\n");
  sleep (5);
  *res = a - b;
  return SOAP_OK;
}

int calcns__mult (struct soap *soap, int a, int b, int *res){  
  printf ("Executing (*)...\n");
  sleep (5);
  *res = a * b;
  return SOAP_OK;
}

int calcns__div (struct soap *soap, int a, int b, int *res){  
  printf ("Executing (/)...\n");
  sleep (5);
  *res = a / b;
  return SOAP_OK;
}



