#include "state_machine.h"

#include <stdlib.h>
#include <stdio.h>

enum STATE next_state(enum STATE state, unsigned char byte, unsigned char control, unsigned char command)
{
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
            return IGNORE;
        }
        break;
    case A_RCV:
        if (byte == command)
        {
            return C_RCV;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return IGNORE;
        }
        break;
    case C_RCV:
        if (byte == (control ^ command))
        {
            return BCC_OK;
        }
        else if (byte == FLAG)
        {
            return FLAG_RCV;
        }
        else
        {
            return IGNORE;
        }
        break;
    case BCC_OK:
        if (byte == FLAG)
        {
            return STOP;
        }
        else
        {
            return IGNORE;
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