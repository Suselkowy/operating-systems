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

void handle_rcv(int serverid);
void zero_mtext();

int running = 1;
int init = 1;
int client_id = 0;
int msqid;
int serverid;

struct msgbuf msg = {COMM_INIT, 0, ""};
struct msgbuf rcv;

void handler(){
    running = 0;
    msg.ntype = COMM_STOP;
    msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);
    exit(EXIT_SUCCESS);
}

void handler_alrm(){
    alarm(0);
    if(running){
        while(msgrcv(msqid, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            handle_rcv(serverid);
        }
    }
    alarm(1);
}

int main(int argc, char const *argv[])
{      
    signal(SIGINT, handler);
    signal(SIGALRM, handler_alrm);

    key_t key = ftok(getenv("HOME"), SERVER_KEY_ID);
    serverid = msgget(key, IPC_EXCL|0666);

    msqid = msgget(IPC_PRIVATE , IPC_CREAT | 0666);

    msg.clientId = msqid;
    msg.ntype = COMM_INIT;

    msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);

    while (init)
    {
        if(msgrcv(msqid, &rcv, sizeof(rcv), 0, IPC_NOWAIT ) != -1){
            if(rcv.clientId == -1){
                fprintf(stderr, "max number of clients connected to server");
                raise(SIGINT);
            }
            client_id = rcv.clientId;
            msg.clientId = client_id;
            init = 0;
        }
    }

    char* wtf;
    char* buff = NULL;
	size_t buff_size;
    alarm(1);
    while (running)
    {
        printf("%d>", client_id);
        fflush(stdout);
		getline(&buff, &buff_size, stdin);

        if(buff_size > 0){
            if(strncmp(buff, "LIST", 4) == 0){
                msg.ntype = COMM_LIST;
                zero_mtext();
                msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);
                int list_present = 0;
                while(!list_present){
                    if(msgrcv(msqid, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
                        if(rcv.ntype == COMM_LIST){
                            list_present = 1;
                        }
                        handle_rcv(serverid);
                    }
                }
            }else if(strncmp(buff, "2ALL", 4) == 0){
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                msg.ntype = COMM_2ALL;
                msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);
                zero_mtext();
            }else if(strncmp(buff, "STOP", 4) == 0){
                msg.ntype = COMM_STOP;
                strcpy(msg.mtext, "");
                msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);
                zero_mtext();
                raise(SIGINT);
            }else if(strncmp(buff, "2ONE", 4) == 0){
                msg.ntype = COMM_2ONE;
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                msgsnd(serverid, &msg, sizeof(msg), IPC_NOWAIT);
                zero_mtext();
            }else if(strncmp(buff, "CHECK", 5) == 0){
                //do nothing
            }else{
                printf("Invalid command\n");
            }
        }

    }

    return 0;
}


void handle_rcv(int serverid){
    int print_prompt = 1;
    if(rcv.ntype == COMM_STOP){
        running = 0;
        raise(SIGINT);
    }else if(rcv.ntype == COMM_2ALL){
        printf("Broadcast message: %s\n", rcv.mtext);
    }else if(rcv.ntype == COMM_2ONE){
        printf("Private message: %s\n", rcv.mtext);
    }else if(rcv.ntype == COMM_LIST){
        print_prompt = 0;
        printf("List of users: %s\n", rcv.mtext);
    }else{
        printf("Invalid message recived\n");
    }
    if(print_prompt){
        printf("%d>", client_id);
        fflush(stdout);
    }
    
    
}

void zero_mtext(){
    memset(msg.mtext, 0, MAX_MESSAGE_LENGTH+1);
}