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

#define BUFFER_SIZE 1200

int main(int argc, char **argv)
{

    // check the number of inputs
    if (argc != 3)
    {
        printf("The number of arguments is incorrect.\n");
        printf("Usage: deliver <server ip address> <server port num>\n");
        return (EXIT_FAILURE);
    }

    char *destination_address = argv[1];
    char *destination_port = argv[2];
    // From now, we will take the address as the host name

    int address_type = 0;

    // Now it is the time to ask what the user want to do
    while (1)
    {
        printf("Please enter your command: \n");
        char command[100];
        char file_address[200];
        scanf("%s", command);
        scanf("%s", file_address);

        // After we have get the user input, validate it
        if (strcmp(command, "ftp") != 0)
        {
            printf("Command not found!\n");
        }
        else
        {
            // the command is correct right now, then check whether the file exist or not
            if (access(file_address, F_OK) == -1)
            {
                // if the file not found, exit
                printf("The file doesn't exist\n");
                return -1;
            }
            else
            {
                printf("The file exist, going to send the YES message\n");
                break;
            }
        }
    }

    // We have already gained the stuff we need, next step is to send the message!

    // Set up the socket connection
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // return error if the socket cannot be created
    if (sockfd < 0)
    {
        perror("socket error");
        return -1;
    }

    struct addrinfo hints; // configuration
    memset(&hints, 0, sizeof hints);
    // hints points to an addrinfo structure that specifies criteria for selecting the socket address structures returned in the list pointed to by res.
    struct addrinfo *serverinfo; // this pointer will point to the result
    hints.ai_family = AF_INET;   // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    // convert the host name to the ip address (DNS operation)
    int rv = getaddrinfo(destination_address, destination_port, &hints, &serverinfo);

    // sent the message "ftp" to the server
    int num_of_bytes_sent = sendto(sockfd, "ftp", 3, 0, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if (num_of_bytes_sent == -1)
    {
        printf("Fail to send 'ftp' to server.\n");
        return -1;
    }
    else
    {
        printf("Successfully sent 'ftp' to the destination server.\n");
    }

    while (1)
    {
        // Then we will need to wait the server return the response to us
        // initialize a buffer that used to store the data recieved from the client
        char buf[BUFFER_SIZE] = {0};
        struct sockaddr_in client_address;          // initialize a container to store client's address so that we can send the information back
        socklen_t len = sizeof(struct sockaddr_in); // get the length of the sockaddr_in, and we will need to use this in bind function

        // try to recieve the message from the client
        int recv_result = recvfrom(sockfd, buf, BUFFER_SIZE, 0, &client_address, &len);

        // check whether the data retrieve is success or not
        if (recv_result < 0)
        {
            perror("Client-Recvfrom: ");
            exit(1);
        }

        // At this time, we have successfully recieved the message from the client
        // We need to check whether the response sent from the client is correct or not
        buf[BUFFER_SIZE] = '\0'; // make the buf become a string so that we can use strcmp

        // Validate the response from the server
        if (strcmp(buf, "yes") == 0)
        {
            printf("A file transfer can start\n");
        }
        else
        {
            printf("Bad Request\n");
            return -1;
        }
    }
}
