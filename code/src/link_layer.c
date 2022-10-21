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

int Nr = 1;
int Ns = 0;

const char *serialPort;
int nRetries;
int timeout;
LinkLayerRole role;

int alarmEnabled = FALSE;
int alarmCount = 0;

int sequence_n;

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
    fd = open(serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
        exit(-1);

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
        exit(-1);

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0.1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;    // Read without blocking
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
        exit(-1);

    if (role == LlTx)
    {
        if (!llopen_tx())
        {
            printf("Error opening connection\n");
            exit(-1);
        }
    }
    else
    {
        if (!llopen_rx())
        {
            printf("Error opening serial port");
            exit(-1);
        }
    }

    return 1;
}

int llopen_tx()
{
    // Mount SET

    unsigned char buf[BUF_SIZE + 1] = {FLAG, A_SENDER, SET, A_SENDER ^ SET, FLAG};

    enum STATE state = START;

    while (alarmCount < nRetries && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            (void)signal(SIGALRM, alarmHandler);

            write(fd, buf, SET_SIZE);
            fprintf(stderr, "SET written\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        int bytes;
        if ((bytes = read(fd, buf, 1)) <= 0)
        {
            if (bytes < 0)
                exit(-1);
            continue;
        }

        // Process byte
        if ((state = next_state(state, *buf, A_SENDER, UA)) == STOP)
        {
            printf("UA received\n");
            break;
        }
    }
    return (state == STOP) ? 1 : -1;
}

int llopen_rx()
{

    // Loop for input
    unsigned char buf[BUF_SIZE + 1]; // +1: Save space for the final '\0' char

    // Receive SET
    enum STATE state = START;
    printf("Waiting for SET...\n");
    while (1)
    {
        int bytes;
        // Returns after 1 chars have been input
        if ((bytes = read(fd, buf, 1)) == 0)
            continue;

        if (state != 0)
            printf("%d and byte received: %d\n", state, bytes);

        // Process byte
        if ((state = next_state(state, *buf, A_SENDER, SET)) == STOP)
        {
            printf("SET received\n");
            break;
        }
    }
    printf("Sending UA...\n");

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
    unsigned char _buf[2 * MAX_PAYLOAD_SIZE];

    int bytes = 0;
    alarmCount = 0;
    alarmEnabled = FALSE;
    enum STATE state = START;

    // Build frame
    int i = 0;
    while (alarmCount < nRetries && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            (void)signal(SIGALRM, alarmHandler);

            // Build frame
            _buf[0] = FLAG;
            _buf[1] = A_SENDER;
            _buf[2] = (Ns << 7);
            _buf[3] = _buf[1] ^ _buf[2];

            // Copy data
            int i, j = 0;
            for (i = 0; i < bufSize; i++)
            {
                // byte stuffing
                if (buf[i] == FLAG || buf[i] == ESCAPE)
                {
                    _buf[4 + i + j] = ESCAPE;
                    _buf[4 + i + j + 1] = 0x5f & buf[i];
                    j++;
                }
                else
                {
                    _buf[4 + i + j] = buf[i];
                }
            }

            // Calculate BCC2
            unsigned char bcc2 = 0;
            for (i = 0; i < bufSize; i++)
            {
                bcc2 ^= buf[i];
            }

            if (bcc2 == FLAG || bcc2 == ESCAPE)
            {
                _buf[4 + i + j] = ESCAPE;
                _buf[4 + i + j + 1] = 0x5f & bcc2;
                j++;
            }
            else
            {
                _buf[4 + i + j] = bcc2;
            }

            _buf[5 + i + j] = FLAG;

            // Write frame
            bytes = write(fd, _buf, 6 + i + j);
            if (bytes < 0)
                exit(-1);

            printf("\nSent %d bytes\n", bytes);

            state = START;

            alarm(timeout);
            alarmEnabled = TRUE;
        }

        // Returns after 1 chars have been input
        read(fd, _buf, 1);

        // Process byte
        state = next_state(state, *_buf, A_SENDER, (RR | (Nr << 7)));
        if (state == STOP)
        {
            printf("RR received\n");
            break;
        }
        i++;
    }

    if (state == STOP)
    {
        Nr = Ns;
        Ns = (Ns + 1) % 2;
        return bytes;
    }
    else
    {
        return -1;
    }
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////

int llread(unsigned char *packet)
{
    unsigned char buf[BUF_SIZE + 1]; // +1: Save space for the final '\0' char

    // Receive packet
    enum STATE state = START;
    while (state != BCC_OK)
    {
        // Returns after 1 chars have been input
        if (read(fd, buf, 1) == 0)
            continue;

        // Process byte
        if (state == A_RCV && (*buf == (0 << 7) || (*buf == (1 << 7))))
        {
            Ns = (*buf >> 7);
            Nr = (Ns + 1) % 2;
        }

        state = next_state(state, *buf, A_SENDER, Ns << 7);
        if (state == IGNORE)
            return 0;
    }

    // Read data
    unsigned char bcc2 = 0;
    int i = 0, data = 0;
    while (state != STOP)
    {
        // Returns after 1 chars have been input
        if (read(fd, buf, 1) == 0)
            continue;

        // If data packet
        if (i == 0)
            data = (*buf == 2);

        // Record sequence number
        if (i == 1 && data)
        {
            if (sequence_n != *buf)
            {
                sequence_n = *buf;
            }
            else
            {
                state = IGNORE;
                printf("Repeated packet\n");
                break;
            }
        }

        if (*buf == FLAG)
        {
            printf("FLAG RCV\n");
            if (bcc2 != 0)
            {
                printf("BCC2 not ok\n");
                state = REJECTED;
                break;
            }
            state = STOP;
            i--;
            *(packet + i) = '\0';
            break;
        }

        if (*buf == ESCAPE)
        {
            // byte destuffing
            read(fd, buf, 1); // read next byte
            if (*buf == 0x5e)
            {
                *buf = FLAG;
            }
            else if (*buf == 0x5d)
            {
                *buf = ESCAPE;
            }
        }

        // Copy data
        *(packet + i) = *buf;
        i++;
        bcc2 ^= *buf;
    }

    // Send RR or REJ
    buf[0] = FLAG;
    buf[1] = A_SENDER;
    buf[2] = (state != REJECTED ? RR : REJ) | (Nr << 7);
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    int bytes = write(fd, buf, RR_SIZE);
    printf("\nWrote RR with bytes:%d\n", bytes);

    if (state == REJECTED || state == IGNORE)
        return 0;

    return i;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{

    // Close serial port
    if (role == LlTx)
    {
        if (!llclose_tx())
        {
            printf("Error closing serial port\n");
            exit(-1);
        }
    }
    else
    {
        if (!llclose_rx())
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

int llclose_tx()
{

    // Mount DISC
    alarmEnabled = FALSE;
    alarmCount = 0;

    unsigned char buf[BUF_SIZE + 1] = {FLAG, A_SENDER, DISC, A_SENDER ^ DISC, FLAG, '\0'};

    enum STATE state = START;
    while (alarmCount < nRetries && state != STOP)
    {
        if (alarmEnabled == FALSE)
        {
            (void)signal(SIGALRM, alarmHandler);

            write(fd, buf, DISC_SIZE);
            fprintf(stderr, "DISC written from tx\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in timeout (s)
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes;
        if ((bytes = read(fd, buf, 1)) <= 0)
        {
            if (bytes < 0)
                exit(-1);
            continue;
        }
        if ((state = next_state(state, *buf, A_RECEIVER, DISC)) == STOP)
        {
            printf("DISC received\n");
        }
    }

    // Send UA
    buf[0] = FLAG;
    buf[1] = A_RECEIVER;
    buf[2] = UA;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    if (!write(fd, buf, UA_SIZE))
    {
        printf("Error sending UA\n");
        exit(-1);
    }

    printf("Sent UA\n");

    return (state == STOP) ? 1 : -1;
}

int llclose_rx()
{
    unsigned char buf[BUF_SIZE + 1] = {0};

    // Receive DISC
    enum STATE state = START;
    while (1)
    {
        // Returns after 1 chars have been input
        read(fd, buf, 1);

        // Process byte
        if ((state = next_state(state, *buf, A_SENDER, DISC)) == STOP)
        {
            printf("DISC received\n");
            break;
        }
    }

    // Send DISC

    // Mount DISC
    unsigned char _buf[BUF_SIZE + 1] = {FLAG, A_RECEIVER, DISC, A_RECEIVER ^ DISC, FLAG, '\0'};
    // Set alarm
    alarmEnabled = FALSE;
    alarmCount = 0;
    state = START;

    while (alarmCount < nRetries && state != STOP)
    {

        if (alarmEnabled == FALSE)
        {
            (void)signal(SIGALRM, alarmHandler);

            write(fd, _buf, DISC_SIZE);
            fprintf(stderr, "DISC written from rx\n");

            state = START;

            alarm(timeout); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        // Read from serial port
        // Returns after 1 chars have been input
        int bytes;
        if ((bytes = read(fd, _buf, 1)) <= 0)
        {
            if (bytes < 0)
                exit(-1);
            continue;
        }

        if ((state = next_state(state, *_buf, A_RECEIVER, UA)) == STOP)
        {
            printf("UA received\n");
            break;
        }
    }

    return (state == STOP) ? 1 : -1;
}
