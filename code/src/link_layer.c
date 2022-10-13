// Link layer protocol implementation

#include "link_layer.h"
#include "state_machine.h"
#include "../cable/cable.c"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

// MISC

struct termios oldtio;
struct termios newtio;
int fd;
int alarmEnabled = FALSE;
int alarmCount = 0;

const char *serialPort;
int nRetries;
int timeout;
LinkLayerRole role;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
    printf("Alarm #%d\n", alarmCount);
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{

    serialPort = connectionParameters.serialPort;
    role = connectionParameters.role;
    nRetries = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;

    int fd = openSerialPort(serialPort, &oldtio, &newtio);

    printf("New termios structure set\n");

    if (role == LlTx)
    {
        return llopen_tx(fd, nRetries, timeout);
    }
    else
    {
        return llopen_rx(fd);
    }
    
    return 1;
}

int llopen_tx(int fd, int nRetransmissions, int timeout)
{
    // Mount SET
    (void)signal(SIGALRM, alarmHandler);

    unsigned char buf[BUF_SIZE + 1] = {FLAG, A_SENDER, SET, A_SENDER ^ SET, FLAG, '\0'};

    enum STATE state = START;

    while (alarmCount < nRetransmissions && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            int bytes = write(fd, buf, SET_SIZE);
            fprintf(stderr, "%d bytes written\n", bytes);

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes = read(fd, buf, 1);
        if (bytes <= 0)
            continue;
        if ((state = next_state(state, *buf)) == STOP)
        {
            printf("UA received\n");
            break;
        }
    }

    return state == STOP;
}

int llopen_rx(int fd){

     // Loop for input
    unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    enum STATE state = START;

    while (1)
    {
        // Returns after 1 chars have been input
        read(fd, buf, 1);

        if ( (state = next_state(state, *buf)) == STOP) {
            printf("SET received\n");
            break;
        }
    }

    buf[0] = FLAG;
    buf[1] = A_SENDER;
    buf[2] = UA;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    int bytes = write(fd, buf, UA_SIZE);
    printf("Sent %d bytes\n", bytes);

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{

    if (role == LlTx)
    {
        return llclose_tx(fd);
    }
    else
    {
        return llclose_rx(fd);
    }

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}

int llclose_tx(int fd) {

    // Mount DISC
    (void)signal(SIGALRM, alarmHandler);
    alarmEnabled = FALSE;
    alarmCount = 0;

    unsigned char buf[BUF_SIZE + 1] = {FLAG, A_SENDER, DISC, A_SENDER ^ DISC, FLAG, '\0'};

    enum STATE state = START;

    while (alarmCount < nRetries && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            int bytes = write(fd, buf, DISC_SIZE);
            fprintf(stderr, "%d bytes written\n", bytes);

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes = read(fd, buf, 1);
        if (bytes <= 0)
            continue;
        if ((state = next_state(state, *buf)) == STOP)
        {
            printf("DISC received\n");
        }
    }

    alarmCount = 0;
    state = START;
    alarmEnabled = FALSE;

    buf[2] = UA;

    if (!write(fd, buf, DISC_SIZE)) {
        return -1;
    }
    printf("Sent DISC\n");

    return state == STOP;
}


int llclose_rx(int fd){

    // Mount DISC
    (void)signal(SIGALRM, alarmHandler);
    alarmEnabled = FALSE;
    alarmCount = 0;

    unsigned char buf[BUF_SIZE + 1] = {FLAG, A_RECEIVER, DISC, A_RECEIVER ^ DISC, FLAG, '\0'};

    enum STATE state = START;

    while (alarmCount < nRetries && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            int bytes = write(fd, buf, DISC_SIZE);
            fprintf(stderr, "%d bytes written\n", bytes);

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes = read(fd, buf, 1);
        if (bytes <= 0)
            continue;
        if ((state = next_state(state, *buf)) == STOP)
        {
            printf("DISC received\n");
            break;
        }
    }

    return state == STOP;
