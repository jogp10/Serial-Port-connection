// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    // TODO

    // Open serial port
    
    if (llopen(serialPort, baudRate, role)) break;

    // Open file

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Could not open file\n");
        exit(1);
    }


    // Send file
    if (strcmp(role, "tx") == 0)
    {
        // TODO
        llwrite(filename);
    }
    // Receive file
    else if (strcmp(role, "rx") == 0)
    {
        // TODO
        llread(filename);
    }
    else
    {
        printf("Invalid role: %s\n", role);
        exit(1);
    }

    
    // Close serial port
    if (llclose(0)) break;

    printf("Application layer protocol finished\n");
}
