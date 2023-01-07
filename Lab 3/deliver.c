#include "helpers.h"

int main(int argc, char **argv)
{

    clock_t start_time;
    clock_t end_time;
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
    printf("Please enter your command: \n");
    char command[100];
    char file_address[200];

    // Now it is the time to ask what the user want to do
    while (1)
    {
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
    start_time = clock();
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

    // Then we will need to wait the server return the response to us
    // initialize a buffer that used to store the data recieved from the client
    char buf[BUFFER_SIZE] = {0};
    struct sockaddr_in client_address;          // initialize a container to store client's address so that we can send the information back
    socklen_t len = sizeof(struct sockaddr_in); // get the length of the sockaddr_in, and we will need to use this in bind function
    

    while (1)
    {

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
            end_time = clock();
            printf("A file transfer can start\n");
            
            break;
        }
        else
        {
            printf("Bad Request\n");
            return -1;
        }
    }

        struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    // Now we can start to send the file to the server
    // First, we need to split the file into different packets
    packet **packets_array = splitFile(file_address);

    // Then we need to send the packets to the server
    for (int i = 0; i < packets_array[0]->total_frag; ++i)
    {
        // convert the packet to a string
        unsigned char *packet_string = malloc(sizeof(unsigned char) * 5000);
        int string_length = packetToString(packets_array[i], packet_string);
        int packet_sent_time = 0;
        int ACK_status = 0;

        // initialize the timeout value to 1 second
        int timeout = 1;

        // printf("packet_string: %s\n", packet_string);
        // printf("string_length: %d\n", string_length);

        getaddrinfo(destination_address, destination_port, &hints, &serverinfo);
        while (ACK_status == 0)
        {

            // after resend the packet over 5 times, we will stop the process
            if (packet_sent_time >= 5)
            {
                printf("The packet is lost because we have resent it over 5 times, file transfer terminated.\n");
                exit(-1);
            }
            // send the packet to the server

            int num_of_bytes_sent_packet = sendto(sockfd, packet_string, string_length, 0, serverinfo->ai_addr, serverinfo->ai_addrlen);
            if (num_of_bytes_sent_packet == -1)
            {
                printf("Fail to send the packet %d to server. File transfer terminated.\n", i + 1);
                return -1;
            }
            else
            {
                packet_sent_time++; // increase the packet sent time
                printf("Packet %d sent, waiting for ACK...\n", i + 1);
                // Then we need to wait for the ACK from the server
                // initialize a buffer that used to store the data recieved from the client
                
                char *buf = waitForACK(sockfd, &client_address, &len, &ACK_status);
               
                if (ACK_status == 0)
                {
                    // if the ACK is not received or timeout, then we need to resend the packet
                    printf("ACK %d not received, resending the packet...\n", i + 1);
                    continue;
                }
                else
                {
                    if (strcmp(buf, "ACK") == 0)
                    {
                        printf("ACK received, packet %d sent successfully\n", i + 1);
                    }
                    else
                    {
                        printf("Wrong ACK received, packet %d sent failed\n", i + 1);
                    }
                }
            }
        }
    }

    // We have already sent all the packets to the server
    printf("################ All packets sent ################\n");
    printf("################ Transportation Summary ################\n");
    printf("Total number of packets sent: %d\n", packets_array[0]->total_frag);
    printf("Total number of packets received: %d\n", packets_array[0]->total_frag);
    printf("Total number of packets lost: 0\n");
    printf("Total number of bytes sent: %d\n", packets_array[0]->total_frag * sizeof(packet));

    printf("The round-trip time is: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    freePacketArray(packets_array, packets_array[0]->total_frag);
}
