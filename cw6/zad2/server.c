#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "config.h"
#include <signal.h>
#include <mqueue.h>

char* list(int* clients, int currClient);
void handle_rcv(char *rcv, char *msg, FILE* log);

mqd_t clients[MAX_NUM_OF_CLIENTS];
int active_clients = 0;

int running = 1;
int currClient = 0;
int msqid;

void init_clients(int* clients){
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        clients[i] = -1;
    }
    
}

void handler(){
    running = 0;
}

int main(int argc, char const *argv[])
{      
    init_clients(clients);
    signal(SIGINT, handler);

    FILE* log = fopen("log.txt", "w+");

    struct mq_attr attr;
    attr.mq_msgsize = MAX_MESSAGE_LENGTH + 1;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_curmsgs = 0;

    mqd_t msqid = mq_open("/kolejkaServera", O_CREAT|O_RDONLY,  0666, &attr);

    printf("Starting server with id = %d\n", msqid);

    char* msg = calloc(1024+1, sizeof(char));
    char* rcv = calloc(MAX_MESSAGE_LENGTH+1, 1);
    unsigned int* priority;

    while (running){   
        struct mq_attr old_attr;
        struct mq_attr new_attr;
        new_attr.mq_flags = O_NONBLOCK;
        mq_setattr(msqid, &new_attr, &old_attr);

        if( mq_receive(msqid, rcv, MAX_MESSAGE_LENGTH+1, NULL) != -1){
            handle_rcv(rcv, msg, log);
        }

        mq_setattr(msqid, &old_attr, NULL);
    }
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        if(clients[i] != -1){    
            sprintf(msg, "%d %d %s", COMM_STOP, -1, " ");
            mq_send(clients[i], msg, MAX_MESSAGE_LENGTH+1, COMM_STOP);  
            mq_close(clients[i]);
        }
    }

    mq_close(msqid);
    mq_unlink("/kolejkaServera");
    fclose(log);
    return 0;
}

char* list(int* clients, int currClient){
    char* ans = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
    int j = 0;
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        if(clients[i] != -1 && i != currClient){
            sprintf(&ans[j], "%d, ", i);
            j+= strlen(&ans[j]);
        }
    }
    return ans;
}

void handle_rcv(char *rcv, char *msg,FILE* log){
    time_t t;
    time(&t);

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    int ntype;
    int clientId;
    char* mtext = calloc(1024, sizeof(char));
    sscanf(rcv, "%d %d %[^\t\n]", &ntype, &clientId, mtext);
    fprintf(log, "%d %d '%s' %s",clientId, ntype, mtext , asctime(timeinfo));
    
    switch (ntype)
    {
        case COMM_INIT:
            int newClientId;
            if(active_clients == MAX_NUM_OF_CLIENTS){
                newClientId = -1;
            }else{
                struct mq_attr attr;
                attr.mq_msgsize = MAX_MESSAGE_LENGTH + 1;
                attr.mq_flags = 0;
                attr.mq_maxmsg = 10;
                attr.mq_curmsgs = 0;

                newClientId = currClient;
                clients[currClient] = mq_open(mtext, O_RDWR,  0666, &attr);

                printf("new connection %d\n", clients[currClient]);
                fflush(stdout);

                ++currClient;
                ++active_clients;
            }
            sprintf(msg, "%d %d %s", COMM_INIT, newClientId, " ");
            mq_send(clients[currClient-1], msg, MAX_MESSAGE_LENGTH+1, COMM_INIT);
            break;
        case COMM_2ALL:
            for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
            {
                if(clients[i] != -1 && i != clientId){
                    sprintf(msg, "%d %d %s", COMM_2ALL, clientId, mtext);
                    mq_send(clients[i], msg, MAX_MESSAGE_LENGTH+1, COMM_2ALL);  
                }
            }
            break;
        case COMM_2ONE:
            int id = atoi(strtok(mtext, " "));
            char* mess = strtok(NULL, "\0");
            
            sprintf(msg, "%d %d %s", COMM_2ONE, clientId, mess); 
            
            if(clients[id] == -1){
                printf("Client does not exists");
            }else{
                mq_send(clients[id], msg, MAX_MESSAGE_LENGTH+1, COMM_2ONE); 
            }
            break;
        case COMM_STOP:
            mq_close(clients[clientId]);
            clients[clientId] = -1;
            active_clients--;
            break;
        case COMM_LIST:
            char* tmp = list(clients, clientId);
            sprintf(msg, "%d %d %s", COMM_LIST, 0, tmp); 
            free(tmp);
            mq_send(clients[clientId], msg, MAX_MESSAGE_LENGTH+1, COMM_LIST); 
            break;
        default:
            printf("invalid message\n");
            break;
    }
}
