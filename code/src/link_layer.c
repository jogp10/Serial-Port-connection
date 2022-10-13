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

    const char *serialPortName = connectionParameters.serialPort;
    LinkLayerRole role = connectionParameters.role;
    int nRetransmissions = connectionParameters.nRetransmissions;
    int timeout = connectionParameters.timeout;

    int fd = openSerialPort(serialPortName, &oldtio, &newtio);

    printf("New termios structure set\n");

    if (role == LlTx)
    {
        return llopen_tx(fd, nRetransmissions, timeout);
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

    unsigned char buf[6] = {FLAG, A_SENDER, SET, A_SENDER ^ SET, FLAG, '\0'};

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
    // TODO

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
