#include "config.h"

void handle_rcv();
void clear_mtext(struct msgbuf* msg);
void close_program();
void set_alarm(int seconds);
void process_message_queue();
void init_connection(int server_messqueue_id, int client_messqueue_id);

int running = 1;
int client_name;
int client_messqueue_id;
int server_messqueue_id;

struct msgbuf msg = {COMM_INIT, 0, ""};
struct msgbuf rcv;

int main(int argc, char const *argv[])
{      
    signal(SIGINT, close_program);
    signal(SIGALRM, process_message_queue);

    key_t key = ftok(getenv("HOME"), SERVER_KEY_ID);
    server_messqueue_id = msgget(key, IPC_EXCL|0666);
    client_messqueue_id = msgget(IPC_PRIVATE , IPC_CREAT | 0666);

    init_connection(server_messqueue_id, client_messqueue_id);
    set_alarm(1);

    char* buff = NULL;
	size_t buff_size;
    while (running)
    {
        printf("%d>", client_name);
		getline(&buff, &buff_size, stdin);

        if(buff_size > 0){
            if(strncmp(buff, "LIST", 4) == 0){
                msg.ntype = COMM_LIST;
                msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);
            }else if(strncmp(buff, "2ALL", 4) == 0){
                msg.ntype = COMM_2ALL;
                clear_mtext(&msg);
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);
            }else if(strncmp(buff, "STOP", 4) == 0){
                msg.ntype = COMM_STOP;
                strcpy(msg.mtext, "");
                msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);
                raise(SIGINT);
            }else if(strncmp(buff, "2ONE", 4) == 0){
                msg.ntype = COMM_2ONE;
                clear_mtext(&msg);
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);
            }else{
                printf("Invalid command\n");
            }
        }

    }

    return 0;
}

void close_program(){
    running = 0;
    msg.ntype = COMM_STOP;
    msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);
    msgctl(client_messqueue_id, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}
void set_alarm(int seconds) {
    alarm(seconds);
}

void process_message_queue(){
    if(running){
        while(msgrcv(client_messqueue_id, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            handle_rcv();
        }
    }
    set_alarm(1);
}

void init_connection(int server_messqueue_id, int client_messqueue_id){
    msg.client_id = client_messqueue_id;
    msg.ntype = COMM_INIT;
    msgsnd(server_messqueue_id, &msg, sizeof(msg), IPC_NOWAIT);

    int is_waiting_for_init_message = 1;
    while (is_waiting_for_init_message)
    {
        if(msgrcv(client_messqueue_id, &rcv, sizeof(rcv), 0, IPC_NOWAIT ) != -1){
            if(rcv.ntype == COMM_INIT){
                if(rcv.client_id == -1){
                    fprintf(stderr, "max number of clients connected to server. Try again later\n");
                    raise(SIGINT);
                }
                msg.client_id = client_name = rcv.client_id;
                is_waiting_for_init_message = 0;
            }
        }
    }
}

void handle_rcv(){
    if(rcv.ntype == COMM_STOP){
        running = 0;
        raise(SIGINT);
    }else if(rcv.ntype == COMM_2ALL){
        printf("\nBroadcast message: %s\n", rcv.mtext);
    }else if(rcv.ntype == COMM_2ONE){
        printf("\nPrivate message: %s\n", rcv.mtext);
    }else if(rcv.ntype == COMM_LIST){
        printf("\nList of users: %s\n", rcv.mtext);
    }else{
        printf("\nInvalid message recived\n");
    }
    printf("%d>", client_name);
    fflush(stdout);
}

void clear_mtext(struct msgbuf* msg){
    memset(msg->mtext, 0, MAX_MESSAGE_LENGTH+1);
    strncpy(msg->mtext, "\0", 1);
}