#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal(char *msg)
{
    char errormsg[100];

    strcpy(errormsg, "[!] FATAL ERROR: ");
    strncat(errormsg, msg, 80);
    perror(errormsg);
}
