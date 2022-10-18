#include "state_machine.h"

#include <stdlib.h>
#include <stdio.h>

enum STATE next_state(enum STATE state, unsigned char byte, unsigned char control, unsigned char nControl)
{
    if (&nControl == NULL)
        nControl = -1;

    switch (state)
    {
    case START:
        if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return START;
        }
        break;
    case FLAG_RCV:
        if (byte == control)
        {
            return A_RCV;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return START;
        }
        break;
    case A_RCV:
        if (byte == SET || byte == UA || byte == DISC || (byte & TS_MASK) == RR || (byte & TS_MASK) == REJ)
        {
            return C_RCV;
        }
        else if ((byte & TI_MASK) == TI)
        {
            return C1_RCV;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return START;
        }
        break;
    case C_RCV:
        if (byte == (control ^ SET) || byte == (control ^ UA) || byte == (control ^ DISC) || byte == (control ^ RR0) || byte == (control ^ RR1) || byte == (control ^ REJ) || byte == (control ^ REJ1))
        {
            return BCC_OK;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return START;
        }
        break;
    case C1_RCV:
        if (byte == (control ^ TI0) || byte == (control ^ TI1))
            return BCC1_OK;
        else if (byte == FLAG)
            return FLAG_RCV;
        else
        {
            return START;
        }
    case BCC_OK:
        if (byte == FLAG)
        {
            return STOP;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return START;
        }
        break;
    case BCC1_OK:
        if (byte)
        { // TODO
            return BCC2_OK;
        }
        else
        {
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