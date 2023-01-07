#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>

typedef struct packet
{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[1000];
} packet;

typedef struct file_info
{
    char *buffer;
    unsigned int size;
} file_info;

#endif