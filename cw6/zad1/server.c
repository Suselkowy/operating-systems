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

char* list(int* clients, int currClient);
void handle_rcv(struct msgbuf *rcv, struct msgbuf *msg, FILE* log);
void zero_mtext(struct msgbuf *msg);

int clients[MAX_NUM_OF_CLIENTS];
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
    
    key_t key = ftok(getenv("HOME"), SERVER_KEY_ID);

    int msqid = msgget(key, IPC_CREAT | 0666);

    printf("Starting server with id = %d\n", msqid);

    struct msgbuf msg = {1,-1,""};
    struct msgbuf rcv;
    while (running)
    {
        if(msgrcv(msqid, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            handle_rcv(&rcv, &msg, log);
        }
    }
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        msg.ntype = COMM_STOP;
        if(clients[i] != -1){
            msgsnd(clients[i], &msg, sizeof(msg), IPC_NOWAIT);        
        }
        zero_mtext(&msg);
    }

    printf("%d", active_clients);
    while(active_clients > 0){
        if(msgrcv(msqid, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            if(rcv.ntype == COMM_STOP){
                msgctl(clients[rcv.clientId], IPC_RMID, NULL);
                clients[rcv.clientId] = -1;
                --active_clients;
                printf("%d active clients \n", active_clients);
            }
        }
    }

    msgctl(msqid, IPC_RMID, NULL);
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

void handle_rcv(struct msgbuf *rcv, struct msgbuf *msg, FILE* log){
    time_t t;
    time(&t);

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    fprintf(log, "%d %d '%s' %s",rcv->clientId, rcv->ntype, rcv->mtext , asctime(timeinfo));

    switch (rcv->ntype)
    {
        case COMM_INIT:
            msg->ntype = COMM_INIT;
            if(active_clients == MAX_NUM_OF_CLIENTS){
                msg->clientId = -1;
            }else{
                msg->clientId = currClient;
                clients[currClient] = rcv->clientId;

                printf("new connection %d\n", clients[currClient]);
                fflush(stdout);

                ++currClient;
                ++active_clients;
            }
            
            msgsnd(rcv->clientId, msg, sizeof(*msg), IPC_NOWAIT);
            zero_mtext(msg);
            break;
        case COMM_2ALL:
            strcpy(msg->mtext, rcv->mtext);
            msg->ntype = COMM_2ALL;
            msg->clientId = rcv->clientId;
            for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
            {
                if(clients[i] != -1 && i != rcv->clientId){
                    msgsnd(clients[i], msg, sizeof(*msg), IPC_NOWAIT);
                    zero_mtext(msg);
                }
            }
            break;
        case COMM_2ONE:
            int id = atoi(strtok(rcv->mtext, " "));
            char* mess = strtok(NULL, "\n");
            strcpy(msg->mtext, mess);
            msg->clientId = rcv->clientId;
            msg->ntype = COMM_2ONE;

            if(clients[id] == -1){
                printf("Client does not exists\n");
            }else{
                msgsnd(clients[id], msg, sizeof(*msg), IPC_NOWAIT);
                zero_mtext(msg);
            }

            strcpy(rcv->mtext, "");
            break;
        case COMM_STOP:
            msgctl(clients[rcv->clientId], IPC_RMID, NULL);
            clients[rcv->clientId] = -1;
            active_clients--;
            break;
        case COMM_LIST:
            msg->clientId = -1;
            msg->ntype = COMM_LIST;
            char* tmp = list(clients, rcv->clientId);
            strcpy(msg->mtext, tmp);
            free(tmp);
            msgsnd(clients[rcv->clientId], msg, sizeof(*msg), IPC_NOWAIT);
            zero_mtext(msg);
            break;
        default:
            printf("invalid message\n");
            break;
    }
}

void zero_mtext(struct msgbuf* msg){
    memset(msg->mtext, 0, MAX_MESSAGE_LENGTH+1);
}