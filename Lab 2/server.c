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

#include "helpers.h"

int main(int argc, char **argv)
{
    // This is used to get the user input from the terminal
    if (argc != 2)
    {
        printf("Incorrect input format, you should only type in the port number!\n");
        printf("Usage: server <server port num>\n");
        exit(1);
    }

    // convert user input to integer
    int port_number = atoi(argv[1]);

    // create the socket object by calling socket(), and it will return a
    // socket file descriptor
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // return error if the socket cannot be created
    if (sockfd < 0)
    {
        perror("socket error");
        return -1;
    }

    // Now we should handle the binding between the socket and the address
    // prepare the variable that will be used for binding
    struct sockaddr_in server_addr;                  // the reason why we use sockaddr_in is because it accepts both ivp 4 and ivp 6
    server_addr.sin_family = AF_INET;                // Use IPV 4
    server_addr.sin_port = htons(port_number);       // convert the host port to network port
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // bind the socket to all local interfaces
    socklen_t len = sizeof(struct sockaddr_in);      // get the length of the sockaddr_in, and we will need to use this in bind function

    // bind the address with the socket
    int bind_result = bind(sockfd, &server_addr, len);

    // check the binding result
    if (bind_result < 0)
    {
        perror("bind error");
        return -1;
    }

    // Socket connection set up! Now we will start listining on the port
    printf("Server: listening on the port %d...\n", port_number);

    // Continously listining on the port, and retrieve the data
    while (1)
    {
        // initialize a buffer that used to store the data recieved from the client
        char buf[BUFFER_SIZE] = {0};
        struct sockaddr_in client_address; // initialize a container to store client's address so that we can send the information back

        // try to recieve the message from the client
        int recv_result = recvfrom(sockfd, buf, BUFFER_SIZE, 0, &client_address, &len);

        // check whether the data retrieve is success or not
        if (recv_result < 0)
        {
            perror("Server-Recvfrom: ");
            exit(1);
        }

        // At this time, we have successfully recieved the message from the client
        // We need to check whether the response sent from the client is correct or not
        buf[BUFFER_SIZE] = '\0'; // make the buf become a string so that we can use strcmp

        if (strcmp(buf, "ftp") == 0)
        {
            // The message is correct
            printf("Message correct!\n");
            // send info back to the client
            int number_of_byte_sent;
            number_of_byte_sent = sendto(sockfd, "yes", 3, 0, &client_address, len);

            // check whether the message is sent or not
            if (number_of_byte_sent < 0)
            {
                perror("Message 'Yes' didn't send: ");
                printf("Failed: message 'Yes' didn't send \n");
            }
            else
            {
                printf("The YES message has been sent back to the client!\n");
                break;
            }
        }
        else
        {
            // The message is incorrect
            printf("Message incorrect!\n");
            // send info back to the client
            int number_of_byte_sent;
            number_of_byte_sent = sendto(sockfd, "no", 2, 0, &client_address, len);

            // check whether the message is sent or not
            if (number_of_byte_sent < 0)
            {
                perror("Message 'No' didn't send: ");
                printf("Failed: message 'No' didn't send!\n");
            }
            else
            {
                printf("The NO message has been sent back to the client!\n");
            }
        }
    }

    // Now we will start the file transfer process
    // receive the file name from the client

    char packet_buf[BUFFER_SIZE] = {0};
    struct sockaddr_in client_address; // initialize a container to store client's address so that we can send the information back

    // recieve the first packet from the client so that we will have a rough idea of this transfer request

    int recv_result = recvfrom(sockfd, packet_buf, BUFFER_SIZE, 0, &client_address, &len);
    // packet_buf[BUFFER_SIZE] = '\0'; // make the buf become a string so that we can use strcmp

    // confirm that the first packet is received
    if (recv_result < 0)
    {
        perror("[Server] First packet didn't receive\n");
        exit(1);
    }
    else
    {
        printf("The first packet has been received!\n");
    }

    sendACK(sockfd, &client_address, len);

    packet *first_packet = analyzePacket(packet_buf);

    // // initialize the packet array
    packet **packets_array = (packet **)malloc(sizeof(packet *) * first_packet->total_frag);

    // // put the first packet into the array
    packets_array[0] = first_packet;

    int current_packet_index = 1;
    while (1)
    {
        // initialize a buffer that used to store the data recieved from the client

        // try to recieve the message from the client
        int recv_result = recvfrom(sockfd, packet_buf, BUFFER_SIZE, 0, &client_address, &len);

        // check whether the data retrieve is success or not
        if (recv_result < 0)
        {
            perror("Packet recieving error");
            exit(1);
        }

        // At this time, we have successfully recieved a packet from the client
        // Convert it to a packet struct
        packet *current_packet = analyzePacket(packet_buf);
        sendACK(sockfd, &client_address, len);

        // put the packet into the array
        packets_array[current_packet->frag_no - 1] = current_packet;

        // The message is incorrect
        printf("Package %d recieved!\n", current_packet->frag_no);
        // send info back to the client
        int number_of_byte_sent;
        number_of_byte_sent = sendto(sockfd, "ACK\0", 2, 0, &client_address, len);

        // check whether the message is sent or not
        if (number_of_byte_sent < 0)
        {
            perror("ACK didn't send: ");
            printf("Failed: message ACK didn't send!\n");
        }
        else
        {
            printf("ACK sent! \n");
        }

        current_packet_index++;

        if (current_packet_index == first_packet->total_frag)
        {
            break;
        }
    }

    printf("File transfer complete!\n");

    saveFile(packets_array, first_packet->total_frag);
    freePacketArray(packets_array, first_packet->total_frag);
}
