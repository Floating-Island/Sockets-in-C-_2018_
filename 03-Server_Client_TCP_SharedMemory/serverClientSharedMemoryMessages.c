#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>//struct hostent
#include <netinet/in.h>//struct sockaddr_in, struct in_addr, INADDR_ANY
#include <arpa/inet.h>//struct in_addr made available
#include <sys/socket.h>//AF_INET
#include <unistd.h>//close()


#include "serverClientSharedMemory.c"
#include "newGets.c"//instead of using gets...



#define MAXDATASIZE 100
#define ENDCONNECTION 0
#define SAVEMESSAGE 1
#define RECEIVEMESSAGES 2
#define NOMESSAGES 3



void clientMessageRequestOptions(int socketFileDescriptor, int atPort);

void sendMessageToServer(int socketFileDescriptor);

struct sharedMessage sharedMessageStructCreation();

void sendMessageStruct(int socketFileDescriptor, struct sharedMessage messageStruct);

void retrieveSavedClientMessage(int fromPort, int socketFileDescriptor);

struct sharedMessage receiveMessageStruct(int socketFileDescriptor);



void serverMessageRequestOptions(int atPort, int socketFileDescriptor, struct memoryManager * manager);

struct sharedMessage receiveMessageRequest(int socketFileDescriptor, struct memoryManager *manager);

int receiveClientsMessageStruct(int socketFileDescriptor, struct sharedMessage* messageStruct);

int saveReceivedMessage(struct sharedMessage messageStruct);

void forwardSavedClientMessage(int fromPort, int socketFileDescriptor);

void serverNotifyNoMessages(int port, int socketFileDescriptor);






void clientMessageRequestOptions(int socketFileDescriptor, int atPort)
{
    printf("\n\nYou're at port: %d\n", atPort);
    printf("\nChoose one of these options:\n    %d = send message.\n    %d = receive saved message.\n    %d = close connection and exit.\n\nYour choice: ", SAVEMESSAGE, RECEIVEMESSAGES, ENDCONNECTION);
    int selectedOption;
    scanf("%d",&selectedOption);
    
    switch(selectedOption)
    {
        case SAVEMESSAGE: //send message to server
        sendMessageToServer(socketFileDescriptor);
        break;
        
        case RECEIVEMESSAGES: //retrieve messages from server
        retrieveSavedClientMessage(atPort, socketFileDescriptor);
        break;

        case ENDCONNECTION:
        close(socketFileDescriptor);
        printf("\nBye.\n");
        exit(0);
        break;

        default:
        printf("\nI don't understand what you want...");
        break;
    }
}

void sendMessageToServer(int socketFileDescriptor)
{
    printf("\nThen, a message will be sent to store at a memory space\n");
    struct sharedMessage messageStruct;
    messageStruct = sharedMessageStructCreation();
    messageStruct.intent = SAVEMESSAGE;
    const int sharedMemorySegmentSize = sizeof(struct sharedMemorySegment);
    sendMessageStruct(socketFileDescriptor, messageStruct);
}

struct sharedMessage sharedMessageStructCreation()
{
    char message[MAXDATASIZE];
    int port;
    struct sharedMessage senderStruct;
    printf("\nInsert the client port (which this message will be associated to when storing it): ");
    scanf("%d", &senderStruct.targetPort);
    getc(stdin);//eliminates cr made in scanf... ¬_¬'
    printf("\nWrite the message to store, associated with port %d. Confirm message with return key:\n    ",senderStruct.targetPort);
    newGets(senderStruct.message, MAXDATASIZE);
    return senderStruct;
}

void sendMessageStruct(int socketFileDescriptor, struct sharedMessage messageStruct)
{
    int numbytesSent;
    if ((numbytesSent=send(socketFileDescriptor, &messageStruct, sizeof(messageStruct), 0)) == -1) 
    {
        perror("\nsend error");
        exit(1);
    }
    else
        printf("\n%d bytes sent.", numbytesSent);
}

void retrieveSavedClientMessage(int fromPort, int socketFileDescriptor)
{   
    printf("\nNext, server will be asked to retrieve a message (if any) stored in the corresponding memory space and associated with port %d.\n", fromPort);
    int numbytesSent;
	struct sharedMessage messageStruct;

    messageStruct.targetPort = fromPort;
    messageStruct.intent = RECEIVEMESSAGES;
    strcpy(messageStruct.message, "");
    sendMessageStruct(socketFileDescriptor, messageStruct);
    
	printf("\nRequest sent, awaiting response...");
    messageStruct = receiveMessageStruct(socketFileDescriptor);
    printf("\nResponse received:\n");
    switch(messageStruct.intent)
    {
        case NOMESSAGES:
        printf("\nNo new messages:\n Server says: %s", messageStruct.message);
        break;

        default:
        printf("\nNew message received:\n Message says: %s", messageStruct.message);
        break;
    }
}

struct sharedMessage receiveMessageStruct(int socketFileDescriptor)
{
    int numbytesReceived;
    struct sharedMessage messageStruct;
    numbytesReceived=recv(socketFileDescriptor, &messageStruct, sizeof(messageStruct), 0);
    switch (numbytesReceived) 
    {
        case -1:
        perror("\nreceive error");
        close(socketFileDescriptor);
        exit(1);
        break;

        case 0:
        printf("\nConnection closed from the other side.\n");
        close(socketFileDescriptor);
        exit(0);
        break;
    }
    printf("\n%d bytes were received.", numbytesReceived);
    return messageStruct;

}





void serverMessageRequestOptions(int atPort, int socketFileDescriptor, struct memoryManager *manager)
{
    struct sharedMessage messageStruct;
    int memoryIdentifier;
    messageStruct = receiveMessageRequest(socketFileDescriptor, manager);
    printf("\nA request has arrived from port: %d", atPort);
    
    switch(messageStruct.intent)
    {
        case SAVEMESSAGE:
        memoryIdentifier = saveReceivedMessage(messageStruct);
        addToMemoryManager(memoryIdentifier, manager);
        break;
        
        case RECEIVEMESSAGES:
        forwardSavedClientMessage(atPort, socketFileDescriptor);
        break;

        default:
        printf("\nUnable to know what the request wants...");
        break;
    }
}



struct sharedMessage receiveMessageRequest(int socketFileDescriptor, struct memoryManager *manager)
{
    struct sharedMessage* messageStruct = NULL;
    messageStruct = malloc(sizeof(struct sharedMessage));
    if(messageStruct == NULL)
    {
        printf("\nCouldn't create the sharedMessage struct...");
        exit(1);
    }
    int response =1;
    printf("\n\nNext, a message received from client will be processed.\n\n");
    response = receiveClientsMessageStruct(socketFileDescriptor, messageStruct);
    if(  response == 0 || response == -1)
    {
        eraseMemoryManagerAndContents(manager);
        exit(response);
    }

    return *messageStruct;
}

int receiveClientsMessageStruct(int socketFileDescriptor, struct sharedMessage* messageStruct)
{
    int numbytesReceived;
    numbytesReceived=recv(socketFileDescriptor, messageStruct, sizeof(struct sharedMessage), 0);
    switch (numbytesReceived) 
    {
        case -1:
        perror("\nreceive error");
        close(socketFileDescriptor);
        break;

        case 0:
        printf("\nConnection closed from the other side.\n");
        close(socketFileDescriptor);
        break;

        default:
        printf("\n%d bytes were received.", numbytesReceived);
        break;
    }
    return numbytesReceived;
}

int saveReceivedMessage(struct sharedMessage messageStruct)
{
    int memory;
    printf("\nClient wants to store a message assigned to memory space of port: %d\n", messageStruct.targetPort);
    const int sharedMemorySegmentSize = sizeof(struct sharedMemorySegment);
	memory = produceTo(messageStruct, sharedMemorySegmentSize);
	printf("\nSaved. message stored to memory space assigned to port %d.\n", messageStruct.targetPort);

    return memory;
}

void forwardSavedClientMessage(int fromPort, int socketFileDescriptor)
{   
    printf("\nClient wants to retrieve message stored at memory space associated with port: %d\n", fromPort);
    int numbytesSent;
	struct sharedMessage messageStruct;
	struct sharedMemorySegment segment;
	const int sharedMemorySegmentSize = sizeof(struct sharedMemorySegment);

	if(existsSharedMemorySegmentWith(fromPort, sharedMemorySegmentSize))
	{
		messageStruct.intent = RECEIVEMESSAGES;
		messageStruct.targetPort = fromPort;
		segment = consumeFrom(fromPort, sharedMemorySegmentSize);
		strcpy(messageStruct.message, segment.message);
        sendMessageStruct(socketFileDescriptor, messageStruct);
	}
	else
	{
        printf("\nNo message stored at memory space associated with port: %d.\n", fromPort);
        serverNotifyNoMessages(fromPort, socketFileDescriptor);
    }
}

void serverNotifyNoMessages(int port, int socketFileDescriptor)
{
    struct sharedMessage messageStruct;
    char notify[MAXDATASIZE] = "No new messages stored in memory space associated with your port.";
    messageStruct.intent = NOMESSAGES;
    strcpy(messageStruct.message, notify);
    messageStruct.targetPort = port;
    sendMessageStruct(socketFileDescriptor, messageStruct);
}