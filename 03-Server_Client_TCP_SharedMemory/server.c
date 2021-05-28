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

#define BACKLOG 5 // Number of connections allowed in queue

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
    myAddress = addressCreationZeroed(AF_INET, SERVER_PORT, inAdressAny);//INADDR_ANY:wild card, means many things. Any IP address listens to this port.
    bindSocketToAddress(socketFileDescriptor, &myAddress);

    listenTo(socketFileDescriptor, BACKLOG);

    while(1)
    {
        printf("\n\nAwaiting new connections...\n\n");
        newConnectionFileDescriptor = acceptNewConnection(socketFileDescriptor, &theirAddress);
        if (newConnectionFileDescriptor != -1)
            newConnectionProcessCreationAndRunning(newConnectionFileDescriptor, &theirAddress);
    }
    return 0;
}



void newConnectionProcessCreationAndRunning(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress)
{
    if(!fork())//A connection was made so a new communication thread is made.
        newConnectionProcess(newConnectionFileDescriptor, theirAddress);
    else
        close(newConnectionFileDescriptor);
}

void newConnectionProcess(int newConnectionFileDescriptor, struct sockaddr_in *theirAddress)
{
    int clientPort = ntohs(theirAddress->sin_port);
    struct memoryManager *manager = NULL;
    manager = createMemoryManager();
    while(1)
    {
        //reception and handling of what the client sends:
        serverMessageRequestOptions(clientPort, newConnectionFileDescriptor, manager);
    }
    eraseMemoryManagerAndContents(manager);
}

