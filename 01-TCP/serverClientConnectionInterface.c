#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>//struct hostent
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()

#define MAXDATASIZE 100

//connection/socket related:

int createSocket();

struct sockaddr_in addressCreationZeroed(short sinFamily, unsigned short sinPort, struct in_addr sinAddress);

void bindSocketToAddress(int socketFileDescriptor, struct sockaddr_in *myAddress);

void listenTo(int socketFileDescriptor, int backlog);

struct in_addr getHostnameAddress(char* argumentValues);

void connectToAddress(int socketFileDescriptor, struct sockaddr_in *theirAddress);

int acceptNewConnection(int socketFileDescriptor, struct sockaddr_in *theirAddress);

//send/receive data:

void sendMessageTo(int socketFileDescriptor);//uses MAXDATASIZE

void receiveMessageFrom(int socketFileDescriptor);//uses MAXDATASIZE

void sendMessagePortRecognized(int socketFileDescriptor, int toPort);//uses MAXDATASIZE

void receiveMessagePortRecognized(int socketFileDescriptor, int toPort);//uses MAXDATASIZE



struct in_addr getHostnameAddress(char* argumentValues)
{
    struct hostent *host;
    if ((host=gethostbyname(argumentValues)) == NULL)// saves host if a domain has been made.
    {
        herror("\ngethostbyname error");
        exit(1);
    }
    return *((struct in_addr *)host->h_addr);
}

int createSocket()
{
    int socketFileDescriptor;
    if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nsocket creation error.");
        exit(1);
    }
    return socketFileDescriptor;
}

struct sockaddr_in addressCreationZeroed(short sinFamily, unsigned short sinPort, struct in_addr sinAddress)
{
    struct sockaddr_in address;

    address.sin_family = sinFamily;
    address.sin_port = htons(sinPort);//transform the integer into a network byte (octet).
    address.sin_addr = sinAddress;
    bzero(&(address.sin_zero), 8);

    return address;
}

void bindSocketToAddress(int socketFileDescriptor, struct sockaddr_in *myAddress)
{
    if (bind(socketFileDescriptor, (struct sockaddr *)myAddress, sizeof(struct sockaddr)) == -1)
    {//^binds everything. links structure with port.
        perror("\nbind error");
        exit(1);
    }
}

void listenTo(int socketFileDescriptor, int backlog)
{
    if (listen(socketFileDescriptor, backlog) == - 1)
    {//^ passive mode. Listens via the socket 'socketFileDescriptor'. BACKLOG is used for the queue. It's blocking, which means that execution isn't resumed unless something is received. 
        perror("\nlisten error");
        exit(1);
    }
}

void connectToAddress(int socketFileDescriptor, struct sockaddr_in *theirAddress)
{
    if (connect(socketFileDescriptor, (struct sockaddr *)theirAddress, sizeof(struct sockaddr)) == -1)
    {
        perror("\nconnect error");
        exit(1);
    }
}

int acceptNewConnection(int socketFileDescriptor, struct sockaddr_in *theirAddress)
{
    int newConnectionFileDescriptor;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    if ((newConnectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)theirAddress, &sinSize)) == -1)
        perror("\nnew connection accept error");// ^if the structure is accepted, we got a connection and newConnectionFileDescriptor is assigned to the conection's socket.
    printf("\nserver: got connection from %s port: %d\n", inet_ntoa(theirAddress->sin_addr), ntohs(theirAddress->sin_port));
    return newConnectionFileDescriptor;
}

void sendMessageTo(int socketFileDescriptor)
{
    //send to server:
    char message[MAXDATASIZE];
    int  numbytesSent;
    printf("\nInsert message to send. Confirm with return key:\n     ");
    gets(message);
    if ((numbytesSent=send(socketFileDescriptor, message, strlen(message) + 1, 0)) == -1) 
    {
        perror("\nsend error");
        exit(1);
    }
    printf("\nSent. %d bytes were sent.\n", numbytesSent);
}

void receiveMessageFrom(int socketFileDescriptor)
{
    char message[MAXDATASIZE];
    int numbytesReceived;
    if ((numbytesReceived=recv(socketFileDescriptor, message, MAXDATASIZE, 0)) == -1)// receives instead of sending.
    {
        perror("\nreceieve error");
        exit(1);
    }
    printf("\nReceived: %s\n", message);
}

void sendMessagePortRecognized(int socketFileDescriptor, int toPort)
{
    int numbytesSent;
    char message[MAXDATASIZE];
    printf("\nInsert message to send to client with port %d. Confirm with return key:\n    ",toPort);
    gets(message);
    if ((numbytesSent = send(socketFileDescriptor, message, strlen(message)+1, 0)) == -1)
        perror("send");
    else
        printf("\nSent. %d bytes were sent to client with port %d .\n", numbytesSent, toPort);
}

void receiveMessagePortRecognized(int socketFileDescriptor, int toPort)
{
    int numbytesReceived;
    char message[MAXDATASIZE];
    if ((numbytesReceived=recv(socketFileDescriptor, message, MAXDATASIZE, 0)) == -1)
    {//^ receives instead of sending
        perror("\nreceive error");
        exit(1);
    }
    message[numbytesReceived] = '\0';  
    printf("\nClient with port %d said: %s\n", toPort, message);
}