#ifndef _HELPER_H_
#define _HELPER_H_
#define BUFFER_SIZE 2200
#define MYPORT "4950" // the port users will be connecting to

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include "packet.h"
#include "helpers.c"

int get_number_of_digits(int number);
char *int_to_string(int number);
int packetToString(packet *packet_item, char *res);
packet **splitFile(char *filename);
void freePacketArray(packet **packets_array, int total_frag);
packet *analyzePacket(char *packet_string);
void saveFile(packet **packets_array, int total_frag);
char *waitForACK(int sockfd, struct sockaddr_in *client_address, socklen_t *len, int* ACK_status);
void sendACK(int sockfd, struct sockaddr_in *client_address, socklen_t len);
#endif