#ifndef DEF_H
#define DEF_H

#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA 1000
#define USERNAME_LENGTH 100
#define PASSWORD_LENGTH 100
#define SESSIONNAME_LENGTH 100
#define BACKLOG 10 // how many pending connections queue will ho
#define MAX_USER_PER_SESSION 10
#define MAX_ARGS 5

// enum for message type
enum message_thYPE
{
    LOGIN = 1,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    LE_ACK,
    LE_NAK,
    NEW_SESS,
    NS_ACK,
    NS_NAK,
    MESSAGE,
    QUERY,
    QU_ACK,
    REGISTER,
    RE_ACK,
    RE_NAK,
    DM,
    DM_ACK,
    DM_NAK,
};

typedef struct user_t user_t;
typedef struct session_t session_t;
typedef struct message_th message_th;
typedef struct connection_t connection_t;
typedef struct user_list user_list;
typedef struct session_list session_list;

struct message_th
{
    unsigned int type;
    unsigned int size;
    char source[USERNAME_LENGTH];
    char data[MAX_DATA];
};

struct session_t
{
    char session_id[SESSIONNAME_LENGTH];
    int user_count;
    user_list *user_list;
    session_t *next;
};

struct connection_t
{
    int socketfd;
};

struct user_t
{
    // this is used to store user's basic information
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];

    // store user's connection information
    connection_t *connection;

    // session information
    session_t *session;

    // use linkedlist to make the life easier
    user_t *next;

    int is_login;
    int fd;
};

struct user_list
{
    user_t *head;
    int user_count;
};

struct session_list
{
    session_t *head;
};
#endif