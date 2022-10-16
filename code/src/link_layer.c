// Link layer protocol implementation

#include "link_layer.h"
#include "state_machine.h"


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

// MISC
struct termios oldtio;
struct termios newtio;

int fd;
int alarmEnabled = FALSE;
int alarmCount = 0;

int Nr = 1;
int Ns = 0;

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
    // Set global variables
    serialPort = connectionParameters.serialPort;
    role = connectionParameters.role;
    nRetries = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;

    // Open serial port
    fd = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0)
        return -1;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
        return -1;

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0.1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Read without blocking
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
        return -1;

    printf("New termios structure set\n");

    if (role == LlTx)
    {
        if(!llopen_tx(fd, nRetries, timeout))
        {
            printf("Error opening connection\n");
            return -1;
        }
    }
    else
    {
        if(!llopen_rx(fd))
        {
            printf("Error opening serial port");
            return -1;
        }
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
            write(fd, buf, SET_SIZE);
            fprintf(stderr, "SET written\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        int bytes;
        if ((bytes = read(fd, buf, 1)) <= 0) {
            if (bytes<0) exit(-1);
            continue;
        }

        // Process byte
        if ((state = next_state(state, *buf)) == STOP)
        {
            printf("UA received\n");
            break;
        }
    }
    return (state == STOP) ? 1 : -1;
}


int llopen_rx(int fd){

     // Loop for input
    unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char


    // Receive SET  
    enum STATE state = START;
    while (1)
    {
        // Returns after 1 chars have been input
        read(fd, buf, 1);

        // Process byte
        if ( (state = next_state(state, *buf)) == STOP) {
            printf("SET received\n");
            break;
        }
    }

    // Send UA
    buf[0] = FLAG;
    buf[1] = A_SENDER;
    buf[2] = UA;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    write(fd, buf, UA_SIZE);
    fprintf(stderr, "UA written\n");

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

    // Close serial port
    if (role == LlTx)
    {
        if (!llclose_tx(fd))
        {
            printf("Error closing serial port\n");
            exit(-1);
        }
    }
    else
    {
        if (!llclose_rx(fd))
        {
            printf("Error closing serial port\n");
            exit(-1);
        }
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
            write(fd, buf, DISC_SIZE);
            fprintf(stderr, "DISC written from tx\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in timeout (s)
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes;
        if ((bytes = read(fd, buf, 1)) <= 0) {
            if (bytes<0) exit(-1);
            continue;
        }
        if ((state = next_state(state, *buf)) == STOP)
        {
            printf("DISC received\n");
        }
    }

    // Send UA
    buf[0] = FLAG;
    buf[1] = A_SENDER;
    buf[2] = UA;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    if (!write(fd, buf, UA_SIZE)) {
        printf("Error sending UA\n");
        exit(-1);
    }

    printf("Sent UA\n");

    return (state == STOP) ? 1 : -1;
}


int llclose_rx(int fd){
    unsigned char buf[BUF_SIZE + 1] = {0};
    

    // Receive DISC
    enum STATE state = START;
    while (1)
    {
        // Returns after 1 chars have been input
        read(fd, buf, 1);

        // Process byte
        if ( (state = next_state(state, *buf)) == STOP) {
            printf("DISC received\n");
            break;
        }
    }

    // Send DISC
    
    // Mount DISC
    unsigned char _buf[BUF_SIZE + 1] = {FLAG, A_RECEIVER, DISC, A_SENDER ^ DISC, FLAG, '\0'};
    // Set alarm
    (void)signal(SIGALRM, alarmHandler);
    alarmEnabled = FALSE;
    alarmCount = 0;
    state = START;

    while (alarmCount < nRetries && state != STOP)
    {   

        if (alarmEnabled == FALSE)
        {

            write(fd, _buf, DISC_SIZE);
            fprintf(stderr, "DISC written from rx\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes;
        if ((bytes = read(fd, _buf, 1)) <= 0) {
            if (bytes<0) exit(-1);
            continue;
        }
        
        if ((state = next_state(state, *_buf)) == STOP)
        {
            printf("UA received\n");
            break;
        }
    }


    return (state == STOP) ? 1 : -1;
}
