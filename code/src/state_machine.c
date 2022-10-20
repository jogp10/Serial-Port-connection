#include "state_machine.h"

#include <stdlib.h>
#include <stdio.h>

enum STATE next_state(enum STATE state, unsigned char byte, unsigned char control, unsigned char command, int nControl)
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
        if (byte == command && nControl == -1)
        {
            return C_RCV;
        }
        else if (byte == command)
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
            return START;
        }
        break;
    case C1_RCV:
        if (byte == (control ^ command))
        {
            return BCC1_OK;
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
    case BCC_OK:
        if (byte == FLAG)
        {
            return STOP;
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
    case BCC2_OK:
        if (byte == FLAG)
        {
            return STOP;
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