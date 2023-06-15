#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_NICK_LENGHT 16
#define MAX_MESSAGE_LENGTH 1024

union addr {
  struct sockaddr_un uni;
  struct sockaddr_in web;
};

typedef struct client {
  union addr addr;
  char nickname[16];
  int sock;
  int addr_len;
  int responding;
  int fd;
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