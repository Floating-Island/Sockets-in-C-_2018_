#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()
#include <sys/wait.h>//waitpid()
#include "serverClientConnectionInterface.c"//contains wrapper functions to handle a connection.

#define SERVER_PORT 3490  // Port from were we listen.
#define BACKLOG 5 // Number of max connections in the queue.

void newConnectionProcessCreationAndRunning(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress);

void newConnectionProcess(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress);


int main()
{
    int socketFileDescriptor, newConnectionFileDescriptor; //we listen in socketFileDescriptor, new connection at newConnectionFileDescriptor
    struct sockaddr_in myAddress;
    struct sockaddr_in theirAddress;
    struct in_addr inAdressAny;

    socketFileDescriptor = createSocket();

    inAdressAny.s_addr = INADDR_ANY;
    myAddress = addressCreationZeroed(AF_INET, SERVER_PORT, inAdressAny);//INADDR_ANY:wild card, means many things. Any IP address listens on this port.
    bindSocketToAddress(socketFileDescriptor, &myAddress);

    listenTo(socketFileDescriptor, BACKLOG);

    while(1)
    {
        newConnectionFileDescriptor = acceptNewConnection(socketFileDescriptor, &theirAddress);
        if (newConnectionFileDescriptor != -1)
            newConnectionProcessCreationAndRunning(newConnectionFileDescriptor, &theirAddress);
    }
    return 0;
}



void newConnectionProcessCreationAndRunning(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress)
{
    if(!fork())//a connection has been made, so we create a new communication thread for it.
        newConnectionProcess(newConnectionFileDescriptor, theirAddress);
    else 
        close(newConnectionFileDescriptor);
}

void newConnectionProcess(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress)
{
    int clientPort = ntohs(theirAddress->sin_port);
    while(1)
    {
        //reception of what the client sends:
        receiveMessagePortRecognized(newConnectionFileDescriptor, clientPort);
        //delivery of message to the client:
        sendMessagePortRecognized(newConnectionFileDescriptor, clientPort);
        //waiting on client:
        printf("\nWaiting on client with port %d...", clientPort);
        while(waitpid(-1, NULL, WNOHANG) > 0);// waits for the client
    }
}

