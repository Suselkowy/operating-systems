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

void handle_rcv(int serverid, char* msg, char* rcv);

int running = 1;
int init = 1;
mqd_t client_id = 0;
mqd_t msqid;
mqd_t serverid;
char* queue_name;


void handler(){
    char* msg = calloc(MAX_MESSAGE_LENGTH+1, sizeof(char));
    running = 0;
    sprintf(msg, "%d %d %s",  COMM_STOP, client_id," ");
    mq_send(serverid, msg, MAX_MESSAGE_LENGTH+1, COMM_STOP);  
    mq_close(msqid);
    mq_close(serverid);
    mq_unlink(queue_name);
    exit(EXIT_SUCCESS);
}

void handler_alarm(){
    char* tmp_msg = calloc(MAX_MESSAGE_LENGTH+1, sizeof(char));
    char* tmp_rcv = calloc(MAX_MESSAGE_LENGTH+1, 1);
    mq_receive(msqid, tmp_rcv, MAX_MESSAGE_LENGTH+1, NULL);
    handle_rcv(serverid, tmp_msg, tmp_rcv);
    const struct sigevent notify = {SIGEV_SIGNAL, SIGALRM};
    mq_notify(msqid, &notify);
}

int main(int argc, char const *argv[])
{      
    char* msg = calloc(MAX_MESSAGE_LENGTH+1, sizeof(char));
    char* rcv = calloc(MAX_MESSAGE_LENGTH+1, 1);
    srand(time(NULL));
    signal(SIGINT, handler);
    signal(SIGALRM, handler_alarm);
    unsigned int* priority;

    struct mq_attr attr;
    attr.mq_msgsize = MAX_MESSAGE_LENGTH + 1;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_curmsgs = 0;

    serverid = mq_open("/kolejkaServera", O_WRONLY,  0666, &attr);

    key_t key = rand();
    queue_name = calloc(128, sizeof(char));
    sprintf(queue_name, "/client-%d", key);

    msqid = mq_open(queue_name, O_CREAT|O_RDWR,  0666, &attr);

    sprintf(msg, "%d %d %s", COMM_INIT, msqid, queue_name);
    mq_send(serverid, msg, MAX_MESSAGE_LENGTH + 1, COMM_INIT);

    while (init)
    {
        if(mq_receive(msqid, rcv, MAX_MESSAGE_LENGTH+1, NULL) != -1){
            int ntype;
            int clientId;
            char* mtext = calloc(1024, sizeof(char));

            sscanf(rcv, "%d %d %s", &ntype, &clientId, mtext);

            if(clientId == -1){
                fprintf(stderr, "max number of clients connected to server");
                raise(SIGINT);
            }
            client_id = clientId;
            init = 0;
        }
    }

    char* buff = NULL;
	size_t buff_size;
    const struct sigevent notify = {SIGEV_SIGNAL, SIGALRM};
    mq_notify(msqid, &notify);

    while (running)
    {
        printf("%d>", client_id);
		getline(&buff, &buff_size, stdin);

        if(buff_size > 0){
            if(strncmp(buff, "LIST", 4) == 0){
                sprintf(msg, "%d %d %s", COMM_LIST, client_id, " ");
                mq_send(serverid, msg, MAX_MESSAGE_LENGTH+1, COMM_LIST);  
            }else if(strncmp(buff, "2ALL", 4) == 0){
                sprintf(msg, "%d %d %s",  COMM_2ALL, client_id, &buff[5]);
                mq_send(serverid, msg, MAX_MESSAGE_LENGTH+1, COMM_2ALL);  
            }else if(strncmp(buff, "STOP", 4) == 0){
                mq_close(msqid);
                mq_close(serverid);
                mq_unlink(queue_name);
                exit(EXIT_SUCCESS);
            }else if(strncmp(buff, "2ONE", 4) == 0){
                sprintf(msg, "%d %d %s\t", COMM_2ONE, client_id, &buff[5]);
                mq_send(serverid, msg, MAX_MESSAGE_LENGTH+1, COMM_2ONE);  
            }else if(strncmp(buff, "CHECK", 5) == 0){
                //do nothing
            }else{
                printf("Invalid command\n");
            }
        }
        

    }

    return 0;
}


void handle_rcv(int serverid, char* msg, char* rcv){
    int ntype;
    int clientId;
    char* mtext = calloc(1024, sizeof(char));
    sscanf(rcv, "%d %d %[^\t\n]", &ntype, &clientId, mtext);
    if(ntype == COMM_STOP){
        running = 0;
        raise(SIGINT);
    }else if(ntype == COMM_2ALL){
        printf("Broadcast message: %s\n", mtext);
    }else if(ntype == COMM_2ONE){
        printf("Private message: %s\n", mtext);
    }else if(ntype == COMM_LIST){
        printf("List of users: %s\n", mtext);
    }else{
        printf("Invalid message recived\n");
    }
    printf("%d>", client_id);
    fflush(stdout);
    free(mtext);
}