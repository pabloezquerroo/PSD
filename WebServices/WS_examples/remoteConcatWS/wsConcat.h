//gsoap wsConcatns service name: wsConcat
//gsoap wsConcatns service style: rpc
//gsoap wsConcatns service encoding: encoded
//gsoap wsConcatns service namespace: urn:wsConcatns


#define IMS_MAX_MSG_SIZE 256

typedef char *xsd__string;

typedef struct tMessage{
	int __size;	
	xsd__string __ptr;	
}wsConcatns__tMessage;

int wsConcatns__remoteConcat (wsConcatns__tMessage msgA, wsConcatns__tMessage msgB, wsConcatns__tMessage* msgC);


