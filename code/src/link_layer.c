// Link layer protocol implementation

#include "link_layer.h"

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
#define _POSIX_SOURCE 1 // POSIX compliant source
#define FALSE 0
#define TRUE 1

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

    (void)signal(SIGALRM, alarmHandler);

    int fd = openSerialPort(serialPortName, oldtio, newtio);

    printf("New termios structure set\n");

    if (role == LlTx)
    {
        //return llopen_tx(fd);
    }
    else
    {
        //return llopen_rx(fd);
    }
    
    return 1;
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
