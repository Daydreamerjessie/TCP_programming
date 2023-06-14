all: 
	g++ servermain.cpp -g -o servermain
	g++ client.cpp -g -o client

servermain:
	g++ servermain.cpp -g -o servermain

client:
	g++ client.cpp -g -omclient
