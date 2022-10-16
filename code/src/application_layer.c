// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer connectionParameters;
    LinkLayerRole r = strcmp(role, "tx") == 0 ? LlTx  : (LlRx);

    // Construct connection parameters
    strcpy(connectionParameters.serialPort, serialPort);
    connectionParameters.role = r;
    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    // Open serial port
    if (!llopen(connectionParameters)) {
        printf("Error opening serial port");
        exit(-1);
    }

    // Open file

    //FILE *file = fopen(filename, "r");


    // Send file
    if (r == LlTx)
    {
        // TODO
        //llwrite(filename);
    }
    // Receive file
    else if (r == LlRx)
    {
        // TODO
        //llread(filename);
    }
    else
    {
        printf("Invalid role: %s\n", role);
        exit(-1);
    }

    // Close serial port
    fprintf(stderr, "Disconnecting!\n");
    if (!llclose(0))
    {
        printf("Error closing serial port\n");
        exit(-1);
    }

    printf("Application layer protocol finished\n");
    exit(0);
}
