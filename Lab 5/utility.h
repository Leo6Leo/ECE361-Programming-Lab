#ifndef UTILITY
#define UTILITY

#include "definitions.h"
extern user_list *userList;
extern session_list *sessionList;
void print_session();
user_t *copy_user_node(user_t *src);
user_list *find_user_list_from_session(char *username);
void initialize_user_list(char *database_name);
int remove_user_from_session_list(char *username);
int find_user_from_session_list(char *username, char *session_name);
void send_online_users_and_sessions(int sockfd, session_list *session_list, user_list *user_list);
session_t *search_session_list(session_list *session_list, char *session_id);
int create_session_and_insert(session_list *session_list, char *session_id, user_t *user_node);
session_list *remove_session_node_from_list(session_list *session_list, char *session_id);
user_t *create_single_user(char *username, char *password);
user_t *find_user_in_list(user_list *user_list, char *username);
bool check_password(user_list *user_list, char *username, char *password);
user_list *remove_user_from_list(user_list *user_list, char *username);
void free_user_list(user_list *user_list);
message_th *analysis_incoming_message(char *data);
message_th *create_message(int type, int size, char *source, char *content);
char *parse_message(message_th *message);
int send_message(int socket, char *data, int message_type);
int send_broadcast_message(int socket, char *data, int message_type, char *username);
void broadcast_message(char *username, char *data, int message_type);
int message_action(message_th *message, int socket_fd, user_list *user_list, session_list *session_list);
void insert_user_node(user_list **user_list, user_t *user_node);
void *get_in_addr(struct sockaddr *sa);
void clear_the_input_buffer();
void print_message(message_th *message);
#endif