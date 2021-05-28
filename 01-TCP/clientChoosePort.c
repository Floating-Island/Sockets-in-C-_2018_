#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()
#include <sys/wait.h>//waitpid()
#include "serverClientConnectionInterface.c"//contains wrapper functions to handle a connection.

#define SERVER_PORT 3490 //server port

void checkCorrectArguments(int);


int main(int argc, char *argv[])
{
    int socketFileDescriptor, myPort;
    struct sockaddr_in theirAddress, myAddress;
    struct in_addr hostAddress, inAdressAny;

    checkCorrectArguments(argc);

    socketFileDescriptor = createSocket();

    myPort = atoi(argv[2]);//port where I listen
    inAdressAny.s_addr = INADDR_ANY;
    myAddress = addressCreationZeroed(AF_INET, myPort, inAdressAny);//INADDR_ANY:wild card, means many things. Any IP address listens to this port.
    bindSocketToAddress(socketFileDescriptor, &myAddress);

    hostAddress = getHostnameAddress(argv[1]);
    theirAddress = addressCreationZeroed(AF_INET, SERVER_PORT, hostAddress);
    connectToAddress(socketFileDescriptor, &theirAddress);

    while(1)
    {
        //send to server:
        sendMessageTo(socketFileDescriptor);

        // wait for server
        printf("\nWaiting server...");
        while(waitpid(-1, NULL, WNOHANG) > 0);

        //reception of what the server sends:
        receiveMessageFrom(socketFileDescriptor);
    }
    close(socketFileDescriptor);

    return 0;
}



void checkCorrectArguments(int argc)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: ./client hostname port\n");//hostname is generally localhost.
        exit(1);
    }
}

