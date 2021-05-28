#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 3490

#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
	int sockfd;
	int host_address_size;
	struct hostent *he;
	struct sockaddr_in their_addr;
	struct	sockaddr_in	host_address;
	unsigned char *address_holder;
	int numBytesReceived, numBytesSent;
	char buffer[256];
	char message[MAXDATASIZE];

	if(argc != 2 )
	{
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}

	if((he=gethostbyname(argv[1])) == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port   = htons(PORT);
	their_addr.sin_addr   = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0',8);


	memset((void*)&host_address, 0, sizeof(host_address));
	host_address.sin_family=AF_INET;
	host_address.sin_port=htons(PORT);
	host_address_size=sizeof(host_address);

	address_holder=(unsigned char*)&host_address.sin_addr.s_addr;
	address_holder[0]=127;
	address_holder[1]=0;
	address_holder[2]=0;
	address_holder[3]=1;

	

	while(1)
	{
		printf("\nInsert a message to send to the client. Confirm with return key: ");
		gets(message);

		if((numBytesSent = sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr)))<0)	
			{// Must use sendto() unless other care is taken. Must explicitly tell where to send each packet of data. Beyond our scope for now. (Hint: Use connect())
			perror("sendto()");
			return 1;
			}
		printf("\n%d bytes were sent.\n", numBytesSent);

		printf("\nWaiting...\n");

		if((numBytesReceived = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr*)&host_address, &host_address_size))<0) // Lee e imprime de donde obtuvo los datos.
			{
			printf("%d", errno);
			perror("recvfrom()");
			return 1;
			}

		printf("\nMessage came from port %d: %s, address %d.%d.%d.%d, bytes: %d.\n", PORT, buffer, address_holder[0], address_holder[1], address_holder[2], address_holder[3], numBytesReceived);
	}

	
	return 0;
}

