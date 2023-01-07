#include "utility.h"
#include "utility.c"

// user_list *userList;
char connected_server_ip[20][MAX_DATA];
message_th *message_list[10000] = {0};
int empty_index = 0;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server <port>\n");
        return -1;
    }

    // convert user input to integer
    const char *restrict port_number = argv[1];

    // create a socket (from Beej)
    struct addrinfo hints, *ai, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get the address info (from Beej)
    int rv;
    if (rv = getaddrinfo(NULL, argv[1], &hints, &ai) != 0)
    {
        fprintf(stderr, "Error: cannot get the address info\n", gai_strerror(rv));
        exit(1);
    }

    // create a socket
    int sockfd;

    for (p = ai; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
        {
            continue;
        }

        // set socket option
        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        // bind the socket to the port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "Failed to bind\n");
        exit(2);
    }

    // listen to the port
    if (listen(sockfd, BACKLOG) == -1)
    {
        fprintf(stderr, "Error: cannot listen to the port\n");
        exit(3);
    }
    printf("The server is listening. Waiting for clients to connect...\n");

    // initialize the session list
    sessionList = malloc(sizeof(session_list));
    sessionList->head = NULL;

    // initialize the user list
    userList = (struct user_list *)malloc(sizeof(struct user_list));
    initialize_user_list("user_db.txt");

    struct sockaddr_storage client_addr;      // connector's address information
    socklen_t addr_size = sizeof client_addr; // size of the address
    char s[MAX_DATA];                         // buffer for client data
    fd_set read_fds, main;

    // add sockfd to main set (from Beej's notes)
    int serverfd = sockfd;
    int fdmax;
    int newfd;
    char buffer[INET6_ADDRSTRLEN]; // buffer for IPV6 addr string
    int bytes;
    user_t *current_user;

    while (1)
    {
        FD_ZERO(&main);
        FD_SET(sockfd, &main);
        fdmax = serverfd;
        user_t *temp = userList->head;
        while (temp != NULL)
        {
            if ((temp->is_login) || (temp->fd > 0))
            {
                if (temp->fd > 0)
                {
                    FD_SET(temp->fd, &main);
                }
                if (fdmax < temp->fd)
                {
                    fdmax = temp->fd;
                }
            }
            temp = temp->next;
        }
        printf("fdmax = %d\n", fdmax);
        if (select(fdmax + 1, &main, NULL, NULL, NULL) == -1)
        {
            fprintf(stderr, " Select\n");
            exit(4);
        }
        if (FD_ISSET(sockfd, &main))
        {
            // printf("here2\n");
            newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
            printf("newfd: %d\n", newfd);
            if (newfd == -1)
            {
                perror("accept\n");
            }
            else
            {
                printf("new connection from %s on socket %d\n", inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), buffer, INET6_ADDRSTRLEN), newfd);
                if ((bytes = recv(newfd, s, MAX_DATA - 1, 0)) <= 0)
                {
                    if (bytes == 0)
                    {
                        printf("0 bytes received: server is hanging up here\n");
                    }
                    else
                    {
                        perror("recv");
                    }
                }
                else
                {
                    printf("current newfd is %d\n", newfd);
                    if(s != NULL){
                        printf("%s\n", s);
                    }
                    message_list[empty_index] = malloc(sizeof(message_th));
                    // message_th *new_msg = malloc(sizeof(message_th));
                    message_list[empty_index] = analysis_incoming_message(s);
                    memset(s, 0, MAX_DATA);
                    int res = message_action(message_list[empty_index], newfd, userList, sessionList);
                    free(message_list[empty_index]);
                    message_list[empty_index] = NULL;
                    empty_index++;
                }
            }
        }
        else
        {
            user_t *temp_user = userList->head;
            while (temp_user != NULL)
            {
                // for(int j = 0; j <= fdmax; j++) {
                int user_fd = temp_user->fd;
                printf("looking at user fd %d\n", user_fd);
                if (FD_ISSET(user_fd, &main))
                {
                    printf("current user_fd is %d\n", user_fd);
                    if ((bytes = recv(user_fd, s, MAX_DATA - 1, 0)) <= 0)
                    {
                        /*
                        if (bytes == 0)
                        {
                            printf("0 bytes received: server is hanging up\n");
                        }
                        else
                        {
                            perror("recv");
                            exit(1);
                        }
                        */
                       temp_user->is_login = 0;
                       temp_user->fd = -1;
                       printf("before removing: ");
                       print_user_list(userList);
                       remove_user_from_session_list(temp_user->username);
                       printf("after removing: ");
                       print_user_list(userList);
                       temp_user = temp_user->next;
                    }
                    else
                    {
                        // printf("message received: %s\n", s);
                        // printf("empty_index = %d\n", empty_index);
                        message_list[empty_index] = (struct message_th *)malloc(sizeof(message_th));
                        // printf("in0\n");
                        message_list[empty_index] = analysis_incoming_message(s);
                        memset(s, 0, MAX_DATA);
                        int res = message_action(message_list[empty_index], user_fd, userList, sessionList);
                        free(message_list[empty_index]);
                        message_list[empty_index] = NULL;
                        empty_index++;
                        temp_user = temp_user->next;
                    }
                }
                else
                {
                    temp_user = temp_user->next;
                }
            }
        }
    }
    close(sockfd);
}
