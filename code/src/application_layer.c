// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // TODO

    // Open serial port


    // Open file


    // Send file
    if (strcmp(role, "tx") == 0)
    {
        // TODO
        //llwrite(filename);
    }
    // Receive file
    else if (strcmp(role, "rx") == 0)
    {
        // TODO
        //llread(filename);
    }
    else
    {
        printf("Invalid role: %s\n", role);
        exit(1);
    }

    
    // Close serial port
    if (llclose(0))
    {
        printf("Error closing serial port\n");
        exit(1);
    }

    printf("Application layer protocol finished\n");
}
