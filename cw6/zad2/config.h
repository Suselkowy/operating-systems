#define MAX_MESSAGE_LENGTH 1024
#define SERVER_KEY_ID 'S'
#define MAX_NUM_OF_CLIENTS 4

struct msgbuf{
    long ntype;
    int client_id;
    char mtext[MAX_MESSAGE_LENGTH];
};

enum COMMANDS{
    COMM_PLACEHOLDER,
    COMM_INIT,
    COMM_LIST,
    COMM_2ALL,
    COMM_2ONE,
    COMM_STOP,
    NUMBER_OF_TYPES       
};

