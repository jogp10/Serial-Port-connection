#include "utils.h"

#include <string.h>

int get_file_size(FILE *file)
{
    int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

int nBytes_to_represent(int n) {
    int i = 0;
    while (n > 0) {
        n = n >> 8;
        i++;
    }
    return i;
}

int mount_control_packet(char *control_packet, int start, int file_size, char *filename) {
    control_packet[0] = start; // START
    // FILE SIZE
    control_packet[1] = 0; // T1
    int l1 = nBytes_to_represent(file_size);
    control_packet[2] = l1; // L1
    control_packet[3] = file_size; // V1

    // FILE NAME
    control_packet[4 + (l1 - 1)] = 1; // T2
    control_packet[5 + (l1 - 1)] = strlen(filename); // L2
    strcpy(control_packet + 6, filename); // V2

    return 1;
}

int mount_data_packet(char *data_packet, char *buffer, int size, int n) {
    data_packet[0] = 1; // DATA
    data_packet[1] = n; // N
    data_packet[2] = size/256; // L2
    data_packet[3] = size%256; // L1
    strcpy(data_packet + 4, buffer); // V2

    return 1;
}
