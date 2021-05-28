#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>//struct hostent
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()




struct in_addr getHostnameAddress(char* argumentValues);

int createSocket();

struct sockaddr_in addressCreationZeroed(short sinFamily, unsigned short sinPort, struct in_addr sinAddress);

void bindSocketToAddress(int socketFileDescriptor, struct sockaddr_in *myAddress);

void listenTo(int socketFileDescriptor, int backlog);

void connectToAddress(int socketFileDescriptor, struct sockaddr_in *theirAddress);

int acceptNewConnection(int socketFileDescriptor, struct sockaddr_in *theirAddress);





struct in_addr getHostnameAddress(char* argumentValues)
{
    struct hostent *host;
    if ((host=gethostbyname(argumentValues)) == NULL)// saves host if a domain was made.
    {
        herror("\ngethostbyname error");
        exit(1);
    }
    return *((struct in_addr *)host->h_addr);
}

int createSocket()
{
    int socketFileDescriptor;
    printf("\nCreating a socket...");
    if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nsocket creation error.");
        exit(1);
    }
    printf("\nSocket %d created.", socketFileDescriptor);
    return socketFileDescriptor;
}

struct sockaddr_in addressCreationZeroed(short sinFamily, unsigned short sinPort, struct in_addr sinAddress)
{
    struct sockaddr_in address;

    address.sin_family = sinFamily;
    address.sin_port = htons(sinPort);//transforms this integer into a network byte (octet).
    address.sin_addr = sinAddress;
    bzero(&(address.sin_zero), 8);
    printf("\nAn address has been created an assigned to port: %d.", ntohs(address.sin_port));
    return address;
}

void bindSocketToAddress(int socketFileDescriptor, struct sockaddr_in *myAddress)
{
    printf("\nBinding socket %d to address with port: %d...", socketFileDescriptor, ntohs(myAddress->sin_port));
    if (bind(socketFileDescriptor, (struct sockaddr *)myAddress, sizeof(struct sockaddr)) == -1)
    {//^binds everything. Associates struct with port
        perror("\nbind error");
        exit(1);
    }
    printf("\nBound socket %d to address with port: %d.", socketFileDescriptor, ntohs(myAddress->sin_port));
}

void listenTo(int socketFileDescriptor, int backlog)
{
    printf("\nTrying to listen to socket %d...", socketFileDescriptor);
    if (listen(socketFileDescriptor, backlog) == - 1)
    {//^ passive mode. Listens through socket socketFileDescriptor. BACKLOG is used for the connection queue. Blocking, execution stops until something is received.
        perror("\nlisten error");
        exit(1);
    }
    printf("\nListening through socket %d.", socketFileDescriptor);
}

void connectToAddress(int socketFileDescriptor, struct sockaddr_in *theirAddress)
{
    printf("\nConnecting to address associated to port %d through socket %d...",ntohs(theirAddress->sin_port), socketFileDescriptor);
    if (connect(socketFileDescriptor, (struct sockaddr *)theirAddress, sizeof(struct sockaddr)) == -1)
    {
        perror("\nconnect error");
        exit(1);
    }
    printf("\nConnected to address associated to port %d through socket %d.",ntohs(theirAddress->sin_port), socketFileDescriptor);
}

int acceptNewConnection(int socketFileDescriptor, struct sockaddr_in *theirAddress)
{
    int newConnectionFileDescriptor;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    if ((newConnectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)theirAddress, &sinSize)) == -1)
        perror("\nnew connection accept error");// ^if the struct is accepted, we got a connection and newConnectionFileDescriptor is associated to the connection socket
    printf("\nConnection accepted from address %s with port: %d\n", inet_ntoa(theirAddress->sin_addr), ntohs(theirAddress->sin_port));
    return newConnectionFileDescriptor;
}