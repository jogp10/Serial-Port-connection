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

    // Open serial port
    if (!llopen(connectionParameters))
    {
        printf("Error opening serial port\n");
        exit(-1);
    }

    FILE *file = fopen(filename, "r");
    FILE *output = fopen("penguin-received.gif", "w");

    // Send file
    if (r == LlTx)
    {
        unsigned char buffer[MAX_PAYLOAD_SIZE + 1], control_packet[MAX_PAYLOAD_SIZE + 1];
        int size = get_file_size(file);

        printf("\nSending START control packet\n");
        mount_control_packet(control_packet, 2, size, filename);

        if (llwrite(control_packet, 5 + nBytes_to_represent(size) + strlen(filename)) == -1)
        {
            printf("Error sending control packet\n");
            llclose(0);
            exit(-1);
        }

        printf("Sending file...\n");
        int n = 0, sz, bytes;
        while ((sz = fread(buffer, 1, MAX_PAYLOAD_SIZE - 4, file)) > 0)
        {
            printf("Mount Data Packet #%d\n", n);
            unsigned char data_packet[MAX_PAYLOAD_SIZE];

            mount_data_packet(data_packet, buffer, sizeof(buffer), n);

            while (1)
            {
                if ((bytes = llwrite(data_packet, sz + 4)) == -1)
                {
                    printf("Error sending data packet\n");
                    llclose(0);
                    exit(-1);
                }

                sleep(1);

                if (bytes > 0)
                    break;
            }

            n++;
        }

        printf("Sending END control packet\n");
        mount_control_packet(control_packet, 3, size, filename);

        if (llwrite(control_packet, 5 + nBytes_to_represent(sz) + strlen(filename)) == -1)
        {
            printf("Error sending control packet\n");
            llclose(0);
            exit(-1);
        }
    }
    else if (r == LlRx)
    {
        unsigned char control_packet[MAX_PAYLOAD_SIZE], data_packet[MAX_PAYLOAD_SIZE];

        printf("Receiving START packet...\n");
        if (llread(control_packet) == -1)
        {
            printf("Error receiving control packet\n");
            llclose(0);
            exit(-1);
        }

        printf("\nReceiving file...\n");
        int bytes;
        while (1)
        {
            printf("Reading Data Packet ");
            if ((bytes = llread(data_packet)) == -1)
            {
                printf("Error receiving data packet\n");
                llclose(0);
                exit(-1);
            }
            if (data_packet[0] == 3)
                break;

            if (bytes > 0) fwrite(data_packet + 4, 1, bytes - 4, output);
        }
    }
    else
    {
        printf("Invalid role: %s\n", role);
        exit(-1);
    }

    printf("\nDisconnecting!\n");
    if (!llclose(0))
    {
        printf("Error closing serial port\n");
        exit(-1);
    }

    printf("Application layer protocol finished\n");
    exit(0);
}
