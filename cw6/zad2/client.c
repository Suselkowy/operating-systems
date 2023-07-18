#include "config.h"

void handle_rcv(struct msgbuf* rcv);
void clear_mtext(struct msgbuf* msg);
void close_program();
void process_message_queue();
void init_connection(int server_messqueue_id, int client_messqueue_id);

int running = 1;
int client_name = 0;
mqd_t client_messqueue_id;
mqd_t server_messqueue_id;
char* client_queue_path;
unsigned int* priority;

int main(int argc, char const *argv[])
{   
    struct msgbuf msg = {COMM_INIT, 0, ""};
    struct msgbuf rcv;
    srand(time(NULL));
    signal(SIGINT, close_program);
    signal(SIGALRM, process_message_queue);

    struct mq_attr attr;
    attr.mq_msgsize = sizeof(rcv);
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_curmsgs = 0;
    server_messqueue_id = mq_open("/kolejkaServera", O_WRONLY,  0666, &attr);

    key_t key = rand();
    client_queue_path = calloc(128, sizeof(char));
    sprintf(client_queue_path, "/client-%d", key);
    client_messqueue_id = mq_open(client_queue_path, O_CREAT|O_RDWR,  0666, &attr);

    init_connection(server_messqueue_id, client_messqueue_id);
    msg.client_id = client_name;

    const struct sigevent notify = {SIGEV_SIGNAL, SIGALRM};
    mq_notify(client_messqueue_id, &notify);

    char* buff = NULL;
	size_t buff_size;
    while (running)
    {
        printf("%d>", client_name);
		getline(&buff, &buff_size, stdin);

        if(buff_size > 0){
            if(strncmp(buff, "LIST", 4) == 0){
                msg.ntype = COMM_LIST;
                mq_send(server_messqueue_id, (char*)&msg, sizeof(msg), COMM_LIST);  
            }else if(strncmp(buff, "2ALL", 4) == 0){
                msg.ntype = COMM_2ALL;
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                mq_send(server_messqueue_id, (char*)&msg, sizeof(msg), COMM_2ALL);  
            }else if(strncmp(buff, "STOP", 4) == 0){
                raise(SIGINT);
            }else if(strncmp(buff, "2ONE", 4) == 0){
                msg.ntype = COMM_2ONE;
                clear_mtext(&msg);
                strncpy(msg.mtext, &buff[5], strlen(&buff[5])-1);
                mq_send(server_messqueue_id, (char*)&msg, sizeof(msg), COMM_2ONE);  
            }else{
                printf("Invalid command\n");
            }
        }
        clear_mtext(&msg);
    }

    return 0;
}

void close_program(){
    running = 0;
    struct msgbuf msg;

    msg.ntype = COMM_STOP;
    msg.client_id = client_name;
    mq_send(server_messqueue_id, (char*)&msg, sizeof(msg), COMM_STOP);  
    mq_close(client_messqueue_id);
    mq_close(server_messqueue_id);
    mq_unlink(client_queue_path);

    exit(EXIT_SUCCESS);
}

void process_message_queue(){
    struct msgbuf rcv;
    if(running){
        mq_receive(client_messqueue_id, (char*)&rcv, sizeof(rcv), NULL);
        handle_rcv(&rcv);
        const struct sigevent notify = {SIGEV_SIGNAL, SIGALRM};
        mq_notify(client_messqueue_id, &notify);
    }
}

void init_connection(int server_messqueue_id, int client_messqueue_id){

    struct msgbuf msg;
    struct msgbuf rcv;
    msg.client_id = client_messqueue_id;
    msg.ntype = COMM_INIT;
    strcpy(msg.mtext, client_queue_path);

    int test = mq_send(server_messqueue_id, (char*)&msg, sizeof(msg), COMM_INIT);
    if(test == -1){
        perror("Error sending inti message\n");
        exit(EXIT_FAILURE);
    }
    int is_waiting_for_init_message = 1;
    while (is_waiting_for_init_message)
    {
        if(mq_receive(client_messqueue_id, (char*)&rcv, sizeof(msg), NULL) != -1){
            if(rcv.ntype == COMM_INIT){
                if(rcv.client_id == -1){
                    fprintf(stderr, "max number of clients connected to server. Try again later\n");
                    raise(SIGINT);
                }
                client_name = rcv.client_id;
                is_waiting_for_init_message = 0;
            }
        }
    }
}

void handle_rcv(struct msgbuf* rcv){
    if(rcv->ntype == COMM_STOP){
        running = 0;
        raise(SIGINT);
    }else if(rcv->ntype == COMM_2ALL){
        printf("\nBroadcast message: %s\n", rcv->mtext);
    }else if(rcv->ntype == COMM_2ONE){
        printf("\nPrivate message: %s\n", rcv->mtext);
    }else if(rcv->ntype == COMM_LIST){
        printf("\nList of users: %s\n", rcv->mtext);
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