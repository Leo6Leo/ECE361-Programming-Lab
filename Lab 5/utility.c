#include "utility.h"
user_list *userList = NULL;
session_list *sessionList = NULL;
// =================== Session ===================

// get the session list and online users list, parse both of them into a string
// and send the string to the client
void print_session()
{
    int count = 0;
    int user_count = 0;
    session_t *temp = sessionList->head;
    while (temp != NULL)
    {
        printf("session %d: %s\n", count, temp->session_id);
        user_t *user = temp->user_list->head;
        while (user != NULL)
        {
            printf(" - User %d: %s\n", user_count, user->username);
            user = user->next;
            user_count++;
        }
        user_count = 0;
        temp = temp->next;
        count++;
    }
}

void send_online_users_and_sessions(int sockfd, session_list *session_list, user_list *user_list)
{
    printf("entered\n");
    char *online_users_and_sessions = (char *)malloc(sizeof(char) * MAX_DATA * 3);
    memset(online_users_and_sessions, 0, sizeof(char) * MAX_DATA * 3);
    char *online_users = (char *)malloc(sizeof(char) * MAX_DATA);
    memset(online_users, 0, sizeof(char) * MAX_DATA);
    char *sessions = (char *)malloc(sizeof(char) * MAX_DATA);
    memset(sessions, 0, sizeof(char) * MAX_DATA);

    // get the online users list

    user_t *current_user = userList->head;
    strcat(online_users, "OLU "); // OLU stands for Online Users, this is the header of the online users list
    if (current_user->is_login)
    {
        strcat(online_users, current_user->username);
        strcat(online_users, ",");
    }
    while (current_user->next != NULL)
    {
        current_user = current_user->next;
        if (current_user->is_login)
        {
            strcat(online_users, current_user->username);
            strcat(online_users, ",");
        }
    }

    // get the session list
    session_t *current_session = session_list->head;
    strcat(sessions, "SES "); // SES stands for Sessions, this is the header of the session list

    if (current_session != NULL)
    {

        strcat(sessions, current_session->session_id);
        strcat(sessions, ",");
        while (current_session->next != NULL)
        {
            current_session = current_session->next;
            strcat(sessions, current_session->session_id);
            strcat(sessions, ",");
        }
    }
    // combine the two lists
    strcat(online_users_and_sessions, online_users);
    strcat(online_users_and_sessions, sessions);

    // send the string to the client
    printf("[Server] Sending list information to client\n");
    send_message(sockfd, online_users_and_sessions, QU_ACK);

    free(online_users_and_sessions);
    free(online_users);
    free(sessions);
}

user_list *find_user_list_from_session(char *username)
{
    session_t *temp = sessionList->head;
    while (temp != NULL)
    {
        user_t *current = temp->user_list->head;
        while (current != NULL)
        {
            if (strcmp(current->username, username) == 0)
            {
                return temp->user_list;
            }
            current = current->next;
        }
        temp = temp->next;
    }
    return NULL;
}

int find_user_from_session_list(char *username, char *session_name)
{
    session_t *temp = sessionList->head;
    while (temp != NULL)
    {
        user_t *current = temp->user_list->head;
        while (current != NULL)
        {
            if (strcmp(current->username, username) == 0)
            {
                if (strcmp(temp->session_id, session_name) == 0)
                {
                    printf("existing session name: %s\n", temp->session_id);
                    return 1;
                }
                else
                {
                    printf("existing session name: %s\n", temp->session_id);
                    printf("new session name: %s\n", session_name);
                    return 2;
                }
            }
            current = current->next;
        }
        temp = temp->next;
    }
    return 0;
}

int remove_user_from_session_list(char *username)
{
    session_t *temp = sessionList->head;
    while (temp != NULL)
    {
        user_t *current = temp->user_list->head;
        user_t *previous = NULL;
        while (current != NULL)
        {
            if (strcmp(current->username, username) == 0)
            {
                if (previous == NULL)
                {
                    temp->user_list->head = temp->user_list->head->next;
                }
                else
                {
                    previous->next = current->next;
                }
                temp->user_count--;
                if (temp->user_count <= 0)
                {
                    remove_session_node_from_list(sessionList, temp->session_id);
                    // free the session
                    printf("[Server] The session is empty, the session has been removed");
                }
                free(current);
                return 1;
            }
            previous = current;
            current = current->next;
        }
        temp = temp->next;
    }
    return 0;
}

// search the session list to see if the session id is already in the list
session_t *search_session_list(session_list *session_list, char *session_id)
{
    session_t *current = session_list->head;
    while (current != NULL)
    {
        if (strcmp(current->session_id, session_id) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

user_t *copy_user_node(user_t *src)
{
    user_t *temp = malloc(sizeof(user_t));
    strcpy(temp->password, src->password);
    strcpy(temp->username, src->username);
    temp->connection = src->connection;
    temp->fd = src->fd;
    temp->is_login = src->is_login;
    temp->next = NULL;
    return temp;
}

// create a new session node with dynamica memory allocation and insert it to the session list
int create_session_and_insert(session_list *session_list, char *session_id, user_t *user_node)
{
    // check if the session id is already in the list
    session_t *search_result = search_session_list(session_list, session_id);
    if (search_result != NULL)
    {
        printf("[Error] The session id is already in the list\n");
        return -1; // error
    }

    session_t *new_session = (session_t *)malloc(sizeof(session_t));
    new_session->user_list = (user_list *)malloc(sizeof(user_list));
    new_session->user_list->head = NULL;
    strcpy(new_session->session_id, session_id);
    user_t *user = copy_user_node(user_node);
    insert_user_node(&(new_session->user_list), user);
    new_session->user_count = 1;
    new_session->next = NULL;

    session_t *current = sessionList->head;
    if (sessionList->head == NULL)
    {
        sessionList->head = new_session;
        print_session();
        return 1;
    }
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_session;

    return 1; // success
}

// remove the session node from the session list
session_list *remove_session_node_from_list(session_list *session_list, char *session_id)
{
    session_t *current = session_list->head;
    session_t *previous = NULL;
    while (current != NULL)
    {
        if (strcmp(current->session_id, session_id) == 0)
        {
            if (previous == NULL)
            {
                session_list->head = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            return session_list;
        }
        previous = current;
        current = current->next;
    }
    return session_list;
}

// =================== User ===================

// create new user and add them into the user list
user_t *create_single_user(char *username, char *password)
{
    user_t *new_user = (user_t *)malloc(sizeof(user_t));
    strcpy(new_user->username, username);
    strcpy(new_user->password, password);
    new_user->is_login = 0;
    new_user->fd = -1;
    new_user->session = NULL;
    new_user->connection = NULL;
    new_user->next = NULL;
    return new_user;
}

user_t *add_user_to_list(char *username, char *password, int socketfd)
{
    user_t *new_user = (user_t *)malloc(sizeof(user_t));
    strcpy(new_user->username, username);
    strcpy(new_user->password, password);
    new_user->is_login = 1;
    new_user->fd = socketfd;
    new_user->session = NULL;
    new_user->connection = NULL;
    new_user->next = NULL;

    // write to user_db file
    char *filename = "user_db.txt";
    printf("going to write to db\n");
    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return NULL;
    }
    // write to the text file
    fprintf(fp, username);
    fprintf(fp, " ");
    fprintf(fp, password);
    fprintf(fp, "\n");
    // close the file
    fclose(fp);
    return new_user;
}

// initialize the user list from the file database
void initialize_user_list(char *database_name)
{
    // open the file
    FILE *fp = fopen(database_name, "r");
    if (fp == NULL)
    {
        printf("Error: cannot open the file user_database.txt");
        exit(1);
    }

    // Initialize the user list object with malloc
    //*list = (struct user_list*)malloc(sizeof(struct user_list));
    // user_t *new_user = malloc(sizeof(struct user_t));

    // read the file line by line
    char line[USERNAME_LENGTH + PASSWORD_LENGTH + 3];
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    while (fgets(line, USERNAME_LENGTH + PASSWORD_LENGTH + 3, fp) != NULL)
    {
        // create a new user node

        // parse the line
        // The format of the line is: username password
        char *token = strtok(line, " ");
        strcpy(username, token);
        token = strtok(NULL, " ");
        token[strlen(token) - 1] = '\0'; // remove the newline character
        strcpy(password, token);

        // add the user node into the user list
        user_t *new_user = create_single_user(username, password);
        if (userList->head == NULL)
        {
            userList->head = new_user;
            // printf("userList head %s\n", userList->head->username);
            userList->user_count++;
        }
        else
        {
            user_t *temp = userList->head;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = new_user;
            // printf("userList new %s\n", temp->next->username);
            userList->user_count++;
        }
    }
    // close the file
    fclose(fp);
}

// insert the user node at the end of the user list
void insert_user_node(user_list **user_list, user_t *user_node)
{
    user_t *current = (*user_list)->head;
    if (current == NULL)
    {
        (*user_list)->head = user_node;
        (*user_list)->user_count++;
        return;
    }
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = user_node;
    (*user_list)->user_count++;
}

// check if the user is in the user list
user_t *find_user_in_list(user_list *user_list, char *username)
{
    user_t *current = user_list->head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// check if the password is correct
bool check_password(user_list *user_list, char *username, char *password)
{
    user_t *current = user_list->head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            if (strcmp(current->password, password) == 0)
            {
                printf("password correct\n");
                return true;
            }
            else
            {
                printf("password incorrect\n");
                return false;
            }
        }
        current = current->next;
    }
    return false;
}

// remove the user from the user list given user's username
user_list *remove_user_from_list(user_list *user_list, char *username)
{
    user_t *current = user_list->head;
    user_t *previous = NULL;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            if (previous == NULL)
            {
                current = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            return user_list;
        }
        previous = current;
        current = current->next;
    }
    return user_list;
}

// free the user list
void free_user_list(user_list *user_list)
{
    user_t *current = user_list->head;
    while (current != NULL)
    {
        user_t *temp = current;
        current = current->next;
        free(temp);
    }
    free(user_list);
}

void print_user_list(user_list *user_list)
{
    user_t *temp = user_list->head;
    while (temp != NULL)
    {
        printf("current user: %s %s %d\n", temp->username, " ", temp->is_login);
        temp = temp->next;
    }
}

// =================== Message ===================
message_th *analysis_incoming_message(char *data)
{
    printf("4The server has received the message: %s\n", data);
    message_th *message = (message_th *)malloc(sizeof(message_th));
    printf("5The server has received the message: %s\n", data);
    // The way that we split the data packet is by :
    // The message format is: type:size:source:content
    char *token = strtok(data, ":");
    message->type = atoi(token);

    token = strtok(NULL, ":");
    message->size = atoi(token);

    token = strtok(NULL, ":");
    strcpy(message->source, token);

    token = strtok(NULL, ":");
    // printf("token is %s\n", token);
    if (token == NULL)
    {
        // printf("empty token\n");
        strcpy(message->data, "");
    }
    else
    {
        strcpy(message->data, token);
    }

    return message;
}

// =================== Message ===================
// print the message sent by the client
void print_message(message_th *message)
{
    printf("The message type is: %d\n", message->type);
    printf("The message size is: %d\n", message->size);
    printf("The message source is: %s\n", message->source);
    printf("The message data is: %s\n", message->data);
}

// creata a new message object with malloc and return the pointer
message_th *create_message(int type, int size, char *source, char *content)
{
    message_th *message = (message_th *)malloc(sizeof(message_th));
    message->type = type;
    message->size = size;
    strcpy(message->source, source);
    strcpy(message->data, content);
    return message;
}

// parse the message object into a string, and make it ready to send
char *parse_message(message_th *message)
{
    char *data = (char *)malloc(sizeof(char) * MAX_DATA);
    sprintf(data, "%d:%d:%s:%s", message->type, message->size, message->source, message->data);
    printf("3The server is ready to send the message: %s\n", message->data);

    // free the message object
    free(message);
    printf("4The server is ready to send the message: %s\n", data);

    return data;
}

// send the message to the client
int send_message(int socket, char *data, int message_thype)
{
    printf("6The server is ready to send the message: %s", data);
    message_th *message = create_message(message_thype, strlen(data), "server", data);
    printf("7The server is ready to send the message: %s", data);
    char *data_to_send = parse_message(message);
    printf("8The server is ready to send the message: %s", data);
    int res = send(socket, data_to_send, strlen(data_to_send), 0);
    free(data_to_send);
    data_to_send = NULL;
    if (res == -1)
    {
        printf("[Server] cannot send the message to the client");
        return -1; // error
    }
    return 1; // success
}

int send_broadcast_message(int socket, char *data, int message_thype, char *username)
{
    message_th *message = create_message(message_thype, strlen(data), username, data);
    char *data_to_send = parse_message(message);
    int res = send(socket, data_to_send, strlen(data_to_send), 0);
    free(data_to_send);
    data_to_send = NULL;
    if (res == -1)
    {
        printf("[Server] cannot send the message to the client");
        return -1; // error
    }
    return 1; // success
}

// the function used to guarentee that all the data is sent, if not resend
void send_message_with_retry(int socket, char *data, int message_thype)
{
    int num_of_retry = 0;
    while (send_message(socket, data, message_thype) == -1)
    {
        num_of_retry++;
        printf("[Error] cannot send the message %s to the client, retrying...", data);
        if (num_of_retry >= 3)
        {
            printf("[Error] Retried too many times, %s has been given up! Program will exit right now.", data);
            exit(-1);
        }
    }

    printf("[Success] Successfully sent the message to the client");
}

// broadcast the message to all the clients in the session
void broadcast_message(char *username, char *data, int message_thype)
{
    // find the client node in the session
    user_list *temp = find_user_list_from_session(username);
    if (temp != NULL)
    {
        user_t *target_user_list = temp->head;
        printf("current user: %s\n", target_user_list->username);
        while (target_user_list != NULL)
        {
            printf("current user: %s\n", target_user_list->username);
            send_broadcast_message(target_user_list->fd, data, message_thype, username);
            target_user_list = target_user_list->next;
        }

        printf("[Success] Successfully attempt to broadcast the message to all the clients in the session");
    }
}

// Create the request based on the message type
int message_action(message_th *message, int socket_fd, user_list *user_list, session_list *session_list)
{
    // query the user list and find the user node
    printf("get into the message action");
    user_t *user_node = find_user_in_list(user_list, message->source);
    if (user_node == NULL)
    {
        if(message->type != REGISTER){
            printf("Error: cannot find the user in the user list");
            char *data = "[Server] Cannot find the user in the user list, cannot proceed the request";
            message_th *new_message = create_message(LO_NAK, strlen(data), "Server", data);
            char *data_to_send = parse_message(new_message);
            send(socket_fd, data_to_send, strlen(data_to_send), 0);
            return LO_NAK;
        }    
    }
    else
    {
        if (user_node->is_login == 1)
        {
            if (message->type == LOGIN || message->type == REGISTER)
            {
                char *data = "[Server] Username exists. Login unsuccessfully\n";
                send_message_with_retry(socket_fd, data, LO_NAK);
                printf("[Unsuccess] Username %s has existed\n", message->source);
                return LO_NAK;
            }
        }
        /*
        else if (user_node->is_login == 0)
        {
            if (message->type == REGISTER)
            {
                char *data = "[Server] Username exists. Register unsuccessfully\n";
                send_message_with_retry(socket_fd, data, RE_NAK);
                printf("[Unsuccess] Username %s has existed\n", message->source);
                return RE_NAK;
            }
        }
        */
        else
        {
            user_node->fd = socket_fd;
            printf("change fd to %d\n", socket_fd);
        }
    }

    // Use a switch case to handle different message type
    switch (message->type)
    {
    case LOGIN:
    {
        printf("login1:%d\n", user_node->is_login);
        if (user_node->is_login == 1)
        {
            printf("1is login\n");
        }
        else
        {
            printf("1not login\n");
        }
        // first check if the user is in the user list
        if (check_password(user_list, message->source, message->data))
        {
            // check if the user is already login
            if (user_node->is_login == 1)
            {
                printf("%p\n", user_node);
                printf("login2:%d\n", user_node->is_login);
                printf("2is login\n");
            }
            else
            {
                printf("2not login\n");
            }
            if (user_node->is_login == 1)
            {
                // send the message to the client
                char *data = "[Server] You are already login";

                // send the message to the client
                send_message(socket_fd, data, LO_NAK);

                return LO_NAK;
            }
            else
            {
                // set the user login status to true
                user_node->is_login += 1;
                // send the message to the client
                char *data = "[Server] Login successfully!\n";

                printf("[Success] %s has successfully login\n", user_node->username);

                if (user_node->is_login)
                {
                    printf("3is login\n");
                }
                else
                {
                    printf("3not login\n");
                }

                // send the message to the client
                send_message(socket_fd, data, LO_ACK);

                printf("login4:%d\n", user_node->is_login);

                return LO_ACK;
            }
        }
        else
        {
            // send the message to the client
            char *data = "[Server] Wrong username or password";

            printf("[Server] Wrong username or password");
            printf("[Server] The username is %s", message->source);
            printf("[Server] The password is %s", message->data);
            printf("[Server] The correct password is %s", user_node->password);

            // send the message to the client
            send_message(socket_fd, data, LO_NAK);

            return LO_NAK;
        }
        break;
    }
    case EXIT:
    {
        user_node->is_login = false;

        remove_user_from_session_list(user_node->username);
        printf("[Success] %s has successfully logout\n", user_node->username);

        // remove the user from the session list
        break;
    }
    case JOIN:
    {
        print_session();
        session_t *current_session = search_session_list(sessionList, message->data);
        if (current_session == NULL)
        {
            char *data = "[Server] Session not existed. Create it first\n";
            send_message_with_retry(socket_fd, data, JN_NAK);
            return JN_NAK;
        }

        // check if the session is full
        if (current_session->user_count >= MAX_USER_PER_SESSION)
        {
            // send the message to the client
            char *data = "[Server] The session is full, cannot join the session\n";
            send_message_with_retry(socket_fd, data, JN_NAK);
            return JN_NAK;
        }

        // check if the user is already in the session
        int res = find_user_from_session_list(message->source, message->data);
        if (res == 1)
        {
            // send the message to the client
            char *data = "[Server] You are already in the session. Cannot join the session\n";
            send_message_with_retry(socket_fd, data, JN_NAK);
            return JN_NAK;
        }

        if (res == 2)
        {
            char *data = "[Server] You are already in a different session. Cannot join the session\n";
            send_message_with_retry(socket_fd, data, JN_NAK);
            return JN_NAK;
        }

        // add the user to the session
        user_t *temp = copy_user_node(user_node);
        insert_user_node(&(current_session->user_list), temp);
        current_session->user_count++;

        // send the message to the client
        char *data = "[Server] Join the session successfully\n";
        print_session();
        send_message_with_retry(socket_fd, data, JN_ACK);
        return JN_ACK;
    }
    case LEAVE_SESS:
    {
        print_session();
        int res = remove_user_from_session_list(message->source);
        if (res == 0)
        {
            char *data = "[Server] You are not in the session. Join the session first\n";
            send_message_with_retry(socket_fd, data, LE_NAK);
            return LE_NAK;
        }
        // remove the user from the session
        print_session();
        char *data = "[Server] Leave the session successfully\n";
        send_message_with_retry(socket_fd, data, LE_ACK);
        return LE_ACK;
    }

    case NEW_SESS:
    {
        // create a new session
        int res = find_user_from_session_list(message->source, message->data);
        if (res == 1)
        {
            char *data = "[Server] You are already in the session. Cannot create the session.\n";
            send_message_with_retry(socket_fd, data, NS_NAK);
            return NS_NAK;
        }
        if (res == 2)
        {
            char *data = "[Server] You are in a different session. Cannot create the session.\n";
            send_message_with_retry(socket_fd, data, NS_NAK);
            return NS_NAK;
        }
        if (res == 0)
        {
            if (create_session_and_insert(session_list, message->data, user_node) < 0)
            {
                // send the message to the client
                char *data = "[Server] Cannot create the session, probably the session name is already taken\n";
                send_message_with_retry(socket_fd, data, NS_ACK);
                return NS_ACK;
            }
            print_session();
            // send the message to the client
            char *data = "[Server] Create the session successfully\n";
            send_message_with_retry(socket_fd, data, NS_ACK);
            return NS_ACK;
        }
    }

    case MESSAGE:
    {
        printf("user node username %s\n", user_node->username);
        printf("message username %s\n", message->source);
        broadcast_message(user_node->username, message->data, message->type);

        printf("[Server] Broadcast the message successfully\n");
        return 1;
    }
    case QUERY:
    {
        // get the list of the session and online users
        send_online_users_and_sessions(socket_fd, session_list, user_list);
        return QU_ACK;
    }
    case REGISTER:
    {
        if(user_node != NULL){
            char *data = "[Server] Username exists. Register unsuccessfully\n";
            send_message_with_retry(socket_fd, data, RE_NAK);
            printf("[Unsuccess] Username %s has existed\n", message->source);
            return RE_NAK;
        }
        user_t *new_user = add_user_to_list(message->source, message->data, socket_fd);
        insert_user_node(&userList, new_user);
        print_user_list(userList);
        char *data = "[Server] Register successfully\n";
        send_message_with_retry(socket_fd, data, RE_ACK);
        return RE_ACK;
    }
    case DM:
    {
        // This is a direct message
        // extract the username and the message, here is the format of the message: username@message
        char *username = strtok(message->data, "@");
        char *data = strtok(NULL, "@");
        printf("\nusername: %s\n", username);
        printf("\ndata: %s\n", data);

        // find the target user in the user list
        user_t *target_user = find_user_in_list(user_list, username);
        if (target_user == NULL)
        {
            // send the message to the client
            char *data = "[Server] The target user is not in the database\n";
            send_message(socket_fd, data, DM_NAK);
            return DM_NAK;
        }

        if(target_user->is_login == false){
            char *data = "[Server] The target user is not online\n";
            send_message(socket_fd, data, DM_NAK);
            return DM_NAK;
        }

        // forward the message to the target user
        // This message format should be message only
        // create the message
        message_th *new_message = malloc(sizeof(message_th));
        new_message->type = DM;
        strcpy(new_message->source, message->source);
        strcpy(new_message->data, data);
        new_message->size = strlen(data);

        // send the message to the target user
        // parse the message to the string
        char *message_string = parse_message(new_message);
        int send_res = send(target_user->fd, message_string, strlen(message_string), 0);
        // free(message_string);
        // free(new_message);
        return DM_ACK;

    }
    default:
        break;
    }

    return 1;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void clear_the_input_buffer()
{
    while ((getchar()) != '\n')
        ;
}