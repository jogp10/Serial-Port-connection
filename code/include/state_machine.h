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

#define ACK         0x7f // 0111 1111

#define RR          0x05 // 0000 0101
#define RR0         0x05 // 0000 0101
#define RR1         0x85 // 1000 0101
#define REJ         0x01 // 0000 0001
#define REJ0        0x01 // 0000 0001
#define REJ1        0x81 // 1000 0001

#define MAX_SIZE    256


enum STATE {
    START,
    STOP,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK
};

enum STATE next_state(enum STATE state, unsigned char byte, unsigned char control) {
    switch (state) {
        case START:
            if (byte == FLAG) {
                return FLAG_RCV;
            }
            else {
                return START;
            }
            break;
        case FLAG_RCV:
            if (byte == control) {
                return A_RCV;
            }
            else if (byte == FLAG) {
                return FLAG_RCV;
            }
            else {
                return START;
            }
            break;
        case A_RCV:
            if (byte == SET || byte == UA || byte == DISC || (byte && ACK == RR) || (byte && ACK == REJ)) {
                return C_RCV;
            }
            else if (byte == FLAG) {
                return FLAG_RCV;
            }
            else {
                return START;
            }
            break;
        case C_RCV:
            if (byte == (control ^ SET) || byte == (control ^ UA) || byte == (control ^ DISC) || byte == (control ^ RR) || byte == (control ^ REJ)) {
                return BCC_OK;
            }
            else if (byte == FLAG) {
                return FLAG_RCV;
            }
            else {
                return START;
            }
            break;
        case BCC_OK:
            if (byte == FLAG) {
                return STOP;
            }
            else {
                return START;
            }
            break;
        case STOP:
            return STOP;
            break;
        default:
            return START;
            break;
    }
}

#endif // STATE_MACHINE_H
