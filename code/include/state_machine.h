#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#define FLAG        0x7e // 0111 1110
#define ESCAPE      0x7d // 0111 1101
#define ESCAPE_MASK 0x20 // 0010 0000

#define SET_SIZE    6
#define UA_SIZE     6
#define DISC_SIZE   6
#define RR_SIZE     6
#define REJ_SIZE    6

#define A_SENDER    0x03 // 0000 0011
#define A_RECEIVER  0x01 // 0000 0001

#define SET         0x03 // 0000 0011
#define DISC        0x0b // 0000 1011

#define UA          0x07 // 0000 0111

#define TS_MASK         0x7f // 0111 1111

#define RR          0x05 // 0000 0101
#define RR0         0x05 // 0000 0101
#define RR1         0x85 // 1000 0101
#define REJ         0x01 // 0000 0001
#define REJ0        0x01 // 0000 0001
#define REJ1        0x81 // 1000 0001

#define TI_MASK    0xbf // 1011 1111

#define TI         0x00 // 0000 0000
#define TI0        0x00 // 0000 0000
#define TI1        0x40 // 0100 0000


#define MAX_SIZE    256


enum STATE {
    START,      // 0
    STOP,       // 1
    FLAG_RCV,   // 2
    A_RCV,      // 3
    C_RCV,      // 4
    C1_RCV,     // 5
    BCC_OK,     // 6
    BCC1_OK,    // 7
    BCC2_OK     // 8
};

enum STATE next_state(enum STATE state, unsigned char byte, unsigned char control, unsigned char command, int nControl);

#endif // STATE_MACHINE_H