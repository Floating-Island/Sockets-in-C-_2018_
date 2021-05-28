#include <stdlib.h>

#define MAXDATASIZE 100

struct sharedMessage
{
    int targetPort;
    int intent;
    char message [MAXDATASIZE];
};//to server.

struct sharedMemorySegment
{
    char message [MAXDATASIZE];
};