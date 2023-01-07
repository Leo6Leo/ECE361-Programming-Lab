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
        i++;
    }

    printf("after token\n");

    return args;
}

// wait for message from the server
void *get_message(void *arg)
{
    printf("get_message\n");
    int sockfd = *(int *)arg;
    char buffer[MAX_DATA];
    int numbytes;

    while (1)
    {
        printf("before recv 1\n");
        if ((numbytes = recv(sockfd, buffer, MAX_DATA - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }
        buffer[numbytes] = '\0';
        // allocate memory for the message
        message_th *message = malloc(sizeof(message_th));
        message = analysis_incoming_message(buffer);
        // print the message
        print_message(message);

        // free the memory
        free(message);
    }
}

int socketfd = -1;
char username[USERNAME_LENGTH];
char sessionID[USERNAME_LENGTH];
pthread_t thread;
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

        // printf("after parse command\n");

        char *action = args[0];
        // printf("after action command\n");

        // use if-else to check the command type
        if (strcmp(action, "/login") == 0 || strcmp(action, "/register") == 0)
        {

            //printf("You are trying to log in\n");
            // login to the server
            message_th *login_message = malloc(sizeof(message_th));
            if(strcmp(action, "/login") == 0){
                login_message->type = LOGIN;
            }else{
                login_message->type = REGISTER;
            }

            // /login(0) username(1) password(2) server_ip(3) server_port(4)

            if (args[1] == NULL || args[2] == NULL || args[3] == NULL || args[4] == NULL)
            {
                printf("Please enter the correct command: /login username password server_ip server_port");
                continue;
            }

            strcpy(login_message->source, args[1]); // username
            strcpy(login_message->data, args[2]);   // password

            // update the username
            strcpy(username, login_message->source);

            // print the username
            printf("the username is %s\n", username);

            login_message->size = strlen(login_message->data);

            // get the address info
            printf("the ip is %s\n", args[3]);
            printf("the port is %s\n", args[4]);
            if (getaddrinfo(args[3], args[4], &hints, &res) != 0)
            {
                printf("Error: cannot get the address info\n");
                strcpy(username, "");
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
                    strcpy(username, "");
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
                strcpy(username, "");
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
                strcpy(username, "");
                free(login_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
            pthread_create(&thread, NULL, get_message, &socketfd);
        }

        // printf("after if\n");

        // if the command is logout
        else if (strcmp(action, "/logout") == 0)
        {

            // create the logout message
            message_th *logout_message = malloc(sizeof(message_th));
            logout_message->type = EXIT;

            printf("You are trying to log out\n");
            // logout from the server

            if (socketfd == -1 || username == NULL)
            {
                printf("[Error] You are not logged in\n");
                free(logout_message);
                continue;
            }

            // update the source
            strcpy(logout_message->source, username);
            logout_message->size = 4;

            // create the logout data packet, the format is: type:size:source:content
            // The format for the content is: password
            char logout_data_packet[MAX_DATA];
            memset(logout_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(logout_data_packet, parse_message(logout_message));

            printf("the logout data packet is %s\n", logout_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, logout_data_packet, strlen(logout_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the logout data packet to the server\n");
                free(logout_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the logout data packet to the server\n");
            // set username to null
            strcpy(username, "");
        }

        else if (strcmp(action, "/joinsession") == 0)
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in; Please log in first\n");
                continue;
            }

            printf("You are joining a session\n");
            message_th *join_session_message = malloc(sizeof(message_th));
            join_session_message->type = JOIN;
            join_session_message->size = strlen(args[1]);
            strcpy(join_session_message->source, username);
            if (args[1] == NULL)
            {
                printf("Please enter the correct command: /joinsession <session ID>");
                continue;
            }
            strcpy(join_session_message->data, args[1]); // session ID
            // update the username
            strcpy(sessionID, join_session_message->data);

            // print the username
            printf("the session ID is %s\n", sessionID);

            char join_session_data_packet[MAX_DATA];
            memset(join_session_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(join_session_data_packet, parse_message(join_session_message));

            printf("the session data packet is %s\n", join_session_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, join_session_data_packet, strlen(join_session_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the session data packet to the server\n");
                // strcpy(sessionID, "");
                free(join_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the session data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                // strcpy(sessionID, "");
                free(join_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        else if (strcmp(action, "/leavesession") == 0)
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in. Please log in first\n");
                continue;
            }

            printf("You are leaving a session\n");
            message_th *leave_session_message = malloc(sizeof(message_th));
            leave_session_message->type = LEAVE_SESS;
            leave_session_message->size = 0;
            strcpy(leave_session_message->source, username);
            strcpy(leave_session_message->data, ""); // session ID

            // print the username
            printf("the session ID is %s\n", sessionID);

            char leave_session_data_packet[MAX_DATA];
            memset(leave_session_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(leave_session_data_packet, parse_message(leave_session_message));

            printf("the leave session data packet is %s\n", leave_session_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, leave_session_data_packet, strlen(leave_session_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the leave session data packet to the server\n");
                // strcpy(sessionID, "");
                free(leave_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the leave session data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                // strcpy(sessionID, "");
                free(leave_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        else if (strcmp(action, "/createsession") == 0)
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in; Failed to create a new session\n");
                continue;
            }

            printf("You are creating a session\n");

            message_th *create_session_message = malloc(sizeof(message_th));

            if (args[1] == NULL)
            {
                printf("Please enter the correct command: /createsession <session ID>");
                continue;
            }
            create_session_message->type = NEW_SESS;
            create_session_message->size = strlen(args[1]);
            strcpy(create_session_message->source, username);
            strcpy(create_session_message->data, args[1]); // session ID
            // update the username
            strcpy(sessionID, create_session_message->data);

            // print the username
            printf("the session ID is %s\n", sessionID);

            char new_session_data_packet[MAX_DATA];
            memset(new_session_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(new_session_data_packet, parse_message(create_session_message));

            printf("the new session data packet is %s\n", new_session_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, new_session_data_packet, strlen(new_session_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the new session data packet to the server\n");
                strcpy(sessionID, "");
                free(create_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the new session data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                strcpy(sessionID, "");
                free(create_session_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        else if (strcmp(action, "/list") == 0)
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in; Failed to get a list\n");
                continue;
            }
            printf("You are getting a list\n");
            message_th *list_message = malloc(sizeof(message_th));
            list_message->type = QUERY;
            list_message->size = 0;
            strcpy(list_message->source, username);
            strcpy(list_message->data, ""); // session ID

            char list_data_packet[MAX_DATA];
            memset(list_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(list_data_packet, parse_message(list_message));

            printf("the list data packet is %s\n", list_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, list_data_packet, strlen(list_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the list data packet to the server\n");
                free(list_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the list data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                // strcpy(sessionID, "");
                free(list_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        else if (strcmp(action, "/dm") == 0)
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in; Failed to get a list\n");
                continue;
            }
            printf("You are trying to send the private message\n");

            if (args[1] == NULL || args[2] == NULL)
            {
                printf("Please enter the correct command: /dm <username> <message>");
                continue;
            }

            // get the arguments
            char *receiver = args[1];
            char *message = args[2];

            // create the private message packet
            message_th *dm_message = malloc(sizeof(message_th));
            dm_message->type = DM;
            strcpy(dm_message->source, username);
            
            // the message should looks like this : <receiver>@<message>, and we will put the message into the data field
            char *dm_message_data = malloc(sizeof(char) * (strlen(receiver) + strlen(message) + 2));
            strcpy(dm_message_data, receiver);
            strcat(dm_message_data, "@");
            strcat(dm_message_data, message);
            strcpy(dm_message->data, dm_message_data);
            dm_message->size = strlen(dm_message->data);



            char dm_data_packet[MAX_DATA];

            // convert the message type to string
            strcpy(dm_data_packet, parse_message(dm_message));

            printf("the dm data packet is %s\n", dm_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, dm_data_packet, strlen(dm_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the dm data packet to the server\n");
                free(dm_message);
                free(dm_message_data);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the dm data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                // strcpy(sessionID, "");
                free(dm_message);
                free(dm_message_data);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        /*
        else if (strcmp(action, "/register") == 0)
        {
            message_th *register_message = malloc(sizeof(message_th));
            register_message->type = REGISTER;

            // /login(0) username(1) password(2) server_ip(3) server_port(4)

            if (args[1] == NULL || args[2] == NULL || args[3] == NULL || args[4] == NULL)
            {
                printf("Please enter the correct command: /register username password server_ip server_port");
                continue;
            }

            strcpy(register_message->source, args[1]); // username
            strcpy(register_message->data, args[2]);   // password

            // update the username
            //strcpy(username, register_message->source);

            // print the username
            //printf("the username is %s\n", username);

            register_message->size = strlen(register_message->data);
            printf("password is %s with a length of %d\n", register_message->data, strlen(register_message->data));
            // get the address info
            printf("the ip is %s\n", args[3]);
            printf("the port is %s\n", args[4]);
            if (getaddrinfo(args[3], args[4], &hints, &res) != 0)
            {
                printf("Error: cannot get the address info\n");
                strcpy(username, "");
                free(register_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }
            if (socketfd == -1){
                socketfd = socket(AF_INET, SOCK_STREAM, 0);
                if (connect(socketfd, res->ai_addr, res->ai_addrlen) == -1)
                {
                    perror("[Error] Connection Failed\n");
                    strcpy(username, "");
                    free(register_message);
                    free(args);
                    clear_the_input_buffer();
                    continue;
                }
                printf("[Client] Connected to the server\n");
            }    
            char register_data_packet[MAX_DATA];
            memset(register_data_packet, 0, MAX_DATA);
            strcpy(register_data_packet, parse_message(register_message));
            // register_data_packet[strlen(register_data_packet)-1] = '\0';
            printf("register message is %s\n", register_data_packet);
            if (send(socketfd, register_data_packet, strlen(register_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the register data packet to the server\n");
                strcpy(username, "");
                free(register_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the register data packet to the server\n");
            printf("the msg sent is %s \n", register_data_packet);
            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                strcpy(username, "");
                free(register_message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
            pthread_create(&thread, NULL, get_message, &socketfd);
        }
        */
        else if (strcmp(action, "/quit") == 0)
        {}

        else if (action[0] != '/')
        {
            if (socketfd == -1)
            {
                perror("[Error] You are not logged in; Failed to send a message\n");
                continue;
            }
            printf("You are sending a message\n");
            message_th *message = malloc(sizeof(message_th));
            message->type = MESSAGE;
            message->size = strlen(action);
            strcpy(message->source, username);
            strcpy(message->data, action); // message

            char message_data_packet[MAX_DATA];
            memset(message_data_packet, 0, MAX_DATA);

            // convert the message type to string
            strcpy(message_data_packet, parse_message(message));

            printf("the message data packet is %s\n", message_data_packet);
            // send the logout data packet to the server
            if (send(socketfd, message_data_packet, strlen(message_data_packet), 0) == -1)
            {
                printf("[Error] Cannot send the message data packet to the server\n");
                free(message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Sent the message data packet to the server\n");

            char response[MAX_DATA];
            memset(response, 0, MAX_DATA);
            if (recv(socketfd, response, MAX_DATA, 0) == -1)
            {
                printf("[Error] Cannot receive the response from the server\n");
                // strcpy(sessionID, "");
                free(message);
                free(args);
                clear_the_input_buffer();
                continue;
            }

            printf("[Client] Received the response from the server\n");
            printf("the response is %s\n", response);
        }

        else
        {
            printf("[Client]Wrong command. Please enter a new command\n");
        }
    }
}