#include <sys/ipc.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <signal.h>//SIGINT
#include <string.h>
#include <stdbool.h>

#include "sharedMessageStruct.c"



#define SERVERFILE "serverSharedFile.c"//used to generate a key


struct memoryManager
{
	int *keysArray;
	size_t arraySize;
	int arrayPosition;
};

struct memoryManager* createMemoryManager();

void addToMemoryManager(int sharedPiece,struct memoryManager *manager);

void eraseMemoryManagerAndContents(struct memoryManager *manager);




int allocateSharedMemorySegmentFor(int targetPort, int sharedMemorySegmentSize);

bool existsSharedMemorySegmentWith(int targetPort, int sharedMemorySegmentSize);

int retrieveSharedMemorySegmentIdWith(int targetPort, int sharedMemorySegmentSize);

struct sharedMemorySegment* attachSharedMemorySegmentStructTo(int segmentId);

int produceTo(struct sharedMessage messageStruct, const int size);

struct sharedMemorySegment consumeFrom(int targetPort, const int sharedMemorySegmentSize);

struct sharedMemorySegment observeFrom(int targetPort, const int sharedMemorySegmentSize);//same as consumeFrom but without using shmctl to erase the address




struct memoryManager* createMemoryManager()
{
	struct memoryManager* manager= NULL;

	manager = malloc(sizeof(struct memoryManager));
	if (manager == NULL)
	{
		printf("\nCouldn't create the shared memory manager...");
		exit(1);
	}
	manager->keysArray = malloc(sizeof(int));
	manager->arraySize = sizeof(manager->keysArray);
	manager->arrayPosition = 0;

	return manager;

}

void addToMemoryManager(int sharedPiece,struct memoryManager *manager)
{
	printf("\nRegistering memory to the shared memory manager...");
	if(manager->arrayPosition == 0)
	{
		manager->keysArray[manager->arrayPosition] = sharedPiece;
	}
	else
	{
		manager->keysArray = realloc(manager->keysArray, sizeof(manager->keysArray) + sizeof(int));
		if(manager->keysArray == NULL)
		{
			printf("\nNot enough memory to contain more data...");
			exit(1);
		}
		manager->keysArray[manager->arrayPosition] = sharedPiece;
		
	}
	manager->arrayPosition++;

	printf("\nMemory registered.");
}

void eraseMemoryManagerAndContents(struct memoryManager* manager)
{
	int position;
	for(position = 0; position < manager->arrayPosition; position++)
		shmctl (manager->keysArray[position], IPC_RMID, 0);
	
	free(manager->keysArray);
	free(manager);
}




int allocateSharedMemorySegmentFor(int targetPort, int sharedMemorySegmentSize)
{
	key_t sharedMemoryKey;
	int segmentId;

	sharedMemoryKey = ftok(SERVERFILE,targetPort);

	if((segmentId = shmget(sharedMemoryKey, sharedMemorySegmentSize,IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR))== -1)
	{
		perror("\nshared memory get error");
		exit(1);
	}
	return segmentId;
}

bool existsSharedMemorySegmentWith(int targetPort, int sharedMemorySegmentSize)
{
	key_t sharedMemoryKey;
	int segmentId;

	sharedMemoryKey = ftok(SERVERFILE,targetPort);

	if((segmentId = shmget(sharedMemoryKey, sharedMemorySegmentSize,S_IRUSR | S_IWUSR))== -1)
		return false;
	return true;
}

int retrieveSharedMemorySegmentIdWith(int targetPort, int sharedMemorySegmentSize)
{
	key_t sharedMemoryKey;

	sharedMemoryKey = ftok(SERVERFILE,targetPort);

	return shmget(sharedMemoryKey, sharedMemorySegmentSize,S_IRUSR | S_IWUSR);
}

struct sharedMemorySegment* attachSharedMemorySegmentStructTo(int segmentId)
{
	return (struct sharedMemorySegment*) shmat(segmentId, 0, 0);
}


int produceTo(struct sharedMessage messageStruct, const int size)
{
	int segmentId;
	struct sharedMemorySegment* memoryAddress;
	const int sharedMemorySegmentSize = sizeof(struct sharedMemorySegment);
	
	if(existsSharedMemorySegmentWith(messageStruct.targetPort, sharedMemorySegmentSize))
	{
		printf("\nA message exist for port: %d. It'll be overwritten...", messageStruct.targetPort);
		segmentId = retrieveSharedMemorySegmentIdWith(messageStruct.targetPort, sharedMemorySegmentSize);
	}
	else
	{
		printf("\nNo memory space exists for port: %d. Creating...", messageStruct.targetPort);
		segmentId = allocateSharedMemorySegmentFor(messageStruct.targetPort, sharedMemorySegmentSize);//uses targetPort as part of the key.
	}

	memoryAddress = attachSharedMemorySegmentStructTo(segmentId);
	
	strcpy(memoryAddress->message, messageStruct.message);

	printf ("\nMessage received saved into memory address: %p.", memoryAddress);

	return segmentId;
}

struct sharedMemorySegment consumeFrom(int targetPort, const int sharedMemorySegmentSize)
{
	int segmentId;
	struct sharedMemorySegment memoryAddress;
	
	segmentId = retrieveSharedMemorySegmentIdWith(targetPort, sharedMemorySegmentSize);

	memoryAddress =*attachSharedMemorySegmentStructTo(segmentId);

	shmctl (segmentId, IPC_RMID, 0);

	return memoryAddress;
}

struct sharedMemorySegment observeFrom(int targetPort, const int sharedMemorySegmentSize)
{
	int segmentId;
	struct sharedMemorySegment memoryAddress;
	
	segmentId = retrieveSharedMemorySegmentIdWith(targetPort, sharedMemorySegmentSize);

	memoryAddress =*attachSharedMemorySegmentStructTo(segmentId);

	return memoryAddress;
}
