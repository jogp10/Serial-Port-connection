#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

int get_file_size(FILE *filename);

int nBytes_to_represent(int n);

int mount_control_packet(unsigned char *control_packet, int start, int file_size, const char *filename);

int mount_data_packet(unsigned char *data_packet, unsigned char *buffer, int size, int n);

#endif // UTILS_H
