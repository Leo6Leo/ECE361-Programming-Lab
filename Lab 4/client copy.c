#include "utility.h"
#include "utility.c"

// used to return an array that store the parameters for the command
char **parse_command(char *command)
{
    // The number_of_args does not include the command itself!
    // /login username password -> num_of_args = 2
    char **args = malloc(MAX_ARGS * sizeof(char *));
    printf("before token\n");
    char *token = strtok(command, " ");

    printf("token: %s\n", token);

    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " ");
        printf("token: %s\n", token);

        i++;
    }

    return args;
}

int socketfd = -1;

int main(int argc, char *argv[])
{
    printf("[Client] The client application has started! Please log in with /login command\n");

    // create a socket
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    while (true)
    {
        char command[MAX_DATA];
        printf("[Client] Please enter a command: ");
        scanf("%[^\n]s", command);
        clear_the_input_buffer();
        char **args = parse_command(command);

        char *action = args[0];

        // use if-else to check the command type
        if (strcmp(action, "/login") == 0)
        {

            printf("You are trying to log in\n");
            // login to the server
            message_th *login_message = malloc(sizeof(message_th));
            login_message->type = LOGIN;

            // /login(0) username(1) password(2) server_ip(3) server_port(4)

            strcpy(login_message->source, args[1]); // username
            strcpy(login_message->data, args[2]);   // password

            login_message->size = strlen(login_message->data);

            // get the address info
            printf("the ip is %s\n", args[3]);
            printf("the port is %s\n", args[4]);
            if (getaddrinfo(args[3], args[4], &hints, &res) != 0)
            {
                printf("Error: cannot get the address info\n");
                free(login_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            if (socketfd == -1)
            {
                // create a socket
                socketfd = socket(AF_INET, SOCK_STREAM, 0);
                if (connect(socketfd, res->ai_addr, res->ai_addrlen) == -1)
                {
                    perror("[Error] Connection Failed\n");
                    free(login_message);
                    free(args);
                    clear_the_input_buffer();
                    continue;
                }

                printf("[Client] Connected to the server\n");
            }

            // create the login data packet, the format is: type:size:source:content
            // The format for the content is: password
            char login_data_packet[MAX_DATA];
            memset(login_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(login_data_packet, parse_message(login_message));

            printf("the login data packet is %s\n", login_data_packet);
            // send the login data packet to the server
            if (send(socketfd, login_data_packet, strlen(login_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the login data packet to the server\n");
                free(login_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the login data packet to the server\n");

            // receive the response from the server
            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                free(login_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }
    }
}
