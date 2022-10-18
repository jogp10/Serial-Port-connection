// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

    printf("New termios structure set\n");

    // Open serial port
    if (!llopen(connectionParameters)) {
        printf("Error opening serial port");
        exit(-1);
    }

    FILE *file = fopen(filename, "r");
    FILE *output = fopen("output.txt", "w");

    // Send file
    if (r == LlTx)
    {
        printf("%d\n", get_file_size(file));
        char buffer[BUF_SIZE - 1], control_packet[BUF_SIZE], data_packet[BUF_SIZE];
        int size;

        // Mount Start Control packet
        mount_control_packet(control_packet, 2, get_file_size(file), filename);

        // llwrite(control_packet, 6 + l1 + strlen(filename));

        // Send file
        int n = 0;
        while ((size = fread(buffer, 1, BUF_SIZE - 1, file)) > 0)
        {
            // Mount data packet
            mount_data_packet(data_packet, buffer, size, n%255);
            n++;

            // llwrite(data_packet, size + 4);
        }

        // Mount End Control packet
        mount_control_packet(control_packet, 3, get_file_size(file), filename);

        // llwrite(control_packet, 6 + l1 + strlen(filename));
    }
    else if (r == LlRx)
    {
        char buffer[BUF_SIZE - 1], control_packet[BUF_SIZE], data_packet[BUF_SIZE];
        int size;

        llread(control_packet);

        // Receive file
        while (llread(data_packet) > 0)
        {
            // Write to file
            fwrite(data_packet + 3, 1, data_packet[2], output);
        }
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
