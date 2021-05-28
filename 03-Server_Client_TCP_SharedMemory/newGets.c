#include <stdio.h>
#include <stdlib.h>

char *newGets(char *string, size_t stringSize);

char *newGets(char *string, size_t stringSize)
{
    /* read a line from standard input and strip the linefeed if any */
    if (fgets(string, stringSize, stdin))
    {
        string[strcspn(string, "\n")] = '\0';
        return string;
    }
    return NULL;
}