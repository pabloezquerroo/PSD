#SSL_LIBS=-lssl -lcrypto
#SSL_FLAGS=-DWITH_OPENSSL

SSL_LIBS=
SSL_FLAGS=

# Add -pthread option
PTHREAD_FLAG=-pthread

all: soapC.c client server

soapC.c:
	soapcpp2 -b -c blackJack.h

client:
	gcc $(SSL_FLAGS) $(PTHREAD_FLAG) -w -o client client.c soapC.c soapClient.c game.c -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

server:
	gcc $(SSL_FLAGS) $(PTHREAD_FLAG) -w -o server server.c soapC.c soapServer.c game.c -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE) 

clean:
	rm client server *.xml *.nsmap *.wsdl *.xsd soapStub.h soapServerLib.* soapH.h soapServer.* soapClientLib.* soapClient.* soapC.*
