#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <signal.h>
#define MAX_NICK_LENGHT 16
#define MAX_MESSAGE_LENGTH 1024

typedef struct client {
  int fd;
  char nickname[16];
} client;

enum COMMANDS{
    COMM_PLACEHOLDER,
    COMM_INIT,
    COMM_LIST,
    COMM_2ALL,
    COMM_2ONE,
    COMM_STOP,
    COMM_PING,
    NUMBER_OF_TYPES       
};

typedef struct message {
  int type;
  char from[MAX_NICK_LENGHT];
  char to[MAX_NICK_LENGHT];
  char message[MAX_MESSAGE_LENGTH];
} message;