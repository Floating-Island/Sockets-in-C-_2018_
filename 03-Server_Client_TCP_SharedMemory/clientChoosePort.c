#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()
#include <sys/wait.h>//waitpid()

#include "serverClientConnection.c"//contains wrapper functions to handle a connection.
#include "serverClientSharedMemoryMessages.c"//shared memory message passing between server/client
#include "serverMacros.c"//SERVER_PORT


void checkCorrectArguments(int);


int main(int argc, char *argv[])
{
    int socketFileDescriptor, myPort;
    struct sockaddr_in theirAddress, myAddress;
    struct in_addr hostAddress, inAdressAny;
    char *recpcion=NULL;

    checkCorrectArguments(argc);

    socketFileDescriptor = createSocket();

    myPort = atoi(argv[2]);//port from where I listen
    inAdressAny.s_addr = INADDR_ANY;
    myAddress = addressCreationZeroed(AF_INET, myPort, inAdressAny);//INADDR_ANY:wild card, means many things. Any IP address listens to this port
    bindSocketToAddress(socketFileDescriptor, &myAddress);

    hostAddress = getHostnameAddress(argv[1]);
    theirAddress = addressCreationZeroed(AF_INET, SERVER_PORT, hostAddress);
    connectToAddress(socketFileDescriptor, &theirAddress);

    while(1)
    {
        //send to server:
        clientMessageRequestOptions(socketFileDescriptor, myPort);
    }
    close(socketFileDescriptor);

    return 0;
}



void checkCorrectArguments(int argc)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: ./client hostname port\n");//hostname is localhost almost every time.
        exit(1);
    }
}

