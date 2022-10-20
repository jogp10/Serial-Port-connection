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
    LinkLayerRole r = strcmp(role, "tx") == 0 ? LlTx : (LlRx);

    // Construct connection parameters
    strcpy(connectionParameters.serialPort, serialPort);
    connectionParameters.role = r;
    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    printf("New termios structure set\n");

    // Open serial port
    if (!llopen(connectionParameters))
    {
        printf("Error opening serial port");
        exit(-1);
    }

    FILE *file = fopen(filename, "r");
    FILE *output = fopen("penguing-received.gif", "w");

    // Send file
    if (r == LlTx)
    {
        char buffer[MAX_PAYLOAD_SIZE + 1], control_packet[MAX_PAYLOAD_SIZE + 1];

        // Mount Start Control packet
        int size = get_file_size(file);
        printf("%d\n", size);
        mount_control_packet(control_packet, 2, size, filename);

        printf("Sending START control packet\n");
        llwrite(control_packet, 5 + nBytes_to_represent(size) + strlen(filename));


        // Send file
        printf("Sending file...\n");
        int n = 0, sz;
        while ((sz = fread(buffer, 1, MAX_PAYLOAD_SIZE - 4, file)) > 0)
        {
            // Mount data packet
            printf("Sending data packet #%d\n", n);
            unsigned char data_packet[MAX_PAYLOAD_SIZE];
            mount_data_packet(data_packet, buffer, sz, n);
            n++;

            llwrite(data_packet, sz + 4);
        }

        // Mount End Control packet
        printf("Sending END control packet\n");
        mount_control_packet(control_packet, 3, size, filename);

        llwrite(control_packet, 5 + nBytes_to_represent(sz) + strlen(filename));
    }
    else if (r == LlRx)
    {
        /*
        unsigned char buffer[BUF_SIZE - 1], control_packet[BUF_SIZE];
        int size;

        llread(control_packet);

        unsigned char data_packet[BUF_SIZE];

        // Receive file
        while (llread(data_packet) > 0)
        {

            // Write to file
            fwrite(data_packet + 3, 1, data_packet[2], output);
        }
        */
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
