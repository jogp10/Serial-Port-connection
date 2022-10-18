#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

int get_file_size(FILE *filename);

int nBytes_to_represent(int n);

int mount_control_packet(char *control_packet, int start, int file_size, char *filename);

int mount_data_packet(char *data_packet, char *buffer, int size, int n);

#endif // UTILS_H
