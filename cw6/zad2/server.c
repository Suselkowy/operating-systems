#include "config.h"

char* list_clients(int* clients_messqueue_id, int curr_client);
void handle_rcv();
void clear_mtext(struct msgbuf *msg);
void init_clients_messqueue_id_array();
void handle_sigint();
void disconnect_clients();
void log_message(struct msgbuf* mess);

int clients_messqueue_id[MAX_NUM_OF_CLIENTS];
mqd_t server_messqueue_id;
int active_clients = 0;
int next_client_id = 0;
int running = 1;
struct mq_attr attr;

struct msgbuf rcv;

int main(int argc, char const *argv[])
{      
    init_clients_messqueue_id_array();
    signal(SIGINT, handle_sigint);

    attr.mq_msgsize = sizeof(rcv);
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_curmsgs = 0;
    server_messqueue_id  = mq_open("/kolejkaServera", O_CREAT|O_RDONLY,  0666, &attr);
    printf("Starting server with id = %d\n", server_messqueue_id);

    while (running){   
        struct mq_attr old_attr;
        struct mq_attr new_attr;
        new_attr.mq_flags = O_NONBLOCK;
        mq_setattr(server_messqueue_id, &new_attr, &old_attr);

        if(mq_receive(server_messqueue_id, (char*)&rcv, sizeof(rcv), NULL) != -1){
            handle_rcv();
        }

        mq_setattr(server_messqueue_id, &old_attr, NULL);
    }

    disconnect_clients();
    mq_close(server_messqueue_id);
    mq_unlink("/kolejkaServera");
    return 0;
}

void disconnect_clients(){
    struct msgbuf msg = {1,-1,""};
    msg.client_id = -1;
    msg.ntype = COMM_STOP;
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        if(clients_messqueue_id[i] != -1){    
            mq_send(clients_messqueue_id[i], (char*)&msg, sizeof(msg), COMM_STOP);  
            mq_close(clients_messqueue_id[i]);
        }
    }

}

char* list_clients(int* clients_messqueue_id, int curr_client){
    char* ans = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
    int string_length = 0;
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        if(clients_messqueue_id[i] != -1 && i != curr_client){
            sprintf(&ans[string_length], "%d, ", i);
            string_length += strlen(&ans[string_length]);
        }
    }
    return ans;
}

void log_message(struct msgbuf* mess){
    FILE* log = fopen("log.txt", "a+");

    time_t rawtime;
    struct tm* timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    fprintf(log, "%d %d '%s' %s",mess->client_id, mess->ntype, mess->mtext , asctime(timeinfo));

    fclose(log);
}

void handle_rcv(){

    log_message(&rcv);

    struct msgbuf msg = {1,-1,""};
    switch (rcv.ntype)
    {
        case COMM_INIT:
            msg.ntype = COMM_INIT;
            int client_desc;

            if(active_clients == MAX_NUM_OF_CLIENTS){
                msg.client_id = -1;
                client_desc = mq_open(rcv.mtext, O_RDWR,  0666, &attr);
            }else{
                msg.client_id = next_client_id;
                client_desc = clients_messqueue_id[next_client_id] = mq_open(rcv.mtext, O_RDWR,  0666, &attr);
                printf("new connection %d\n", next_client_id);

                ++next_client_id;
                ++active_clients;
            }

            mq_send(client_desc, (char*)&msg, sizeof(msg), COMM_INIT);

            if(msg.client_id == -1){
                mq_close(client_desc);
            }
            break;
        case COMM_2ALL:
            msg.ntype = COMM_2ALL;
            msg.client_id = rcv.client_id;
            strcpy(msg.mtext, rcv.mtext);
            
            for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
            {
                if(clients_messqueue_id[i] != -1 && i != rcv.client_id){
                    mq_send(clients_messqueue_id[i], (char*)&msg, sizeof(msg), COMM_2ALL);  
                }
            }

            break;
        case COMM_2ONE:
            msg.ntype = COMM_2ONE;
            msg.client_id = rcv.client_id;

            char* id_str = strtok(rcv.mtext, " ");
            if(id_str == NULL){
                perror("Unable to send message.No client id proided.\n");
                break;
            }
            int id = atoi(id_str);
            char* mess = strtok(NULL, "\n");
            if(mess == NULL){
                strcpy(msg.mtext, "");
            }else{
                strcpy(msg.mtext, mess);
            }
        
            if(clients_messqueue_id[id] == -1){
                printf("Client does not exists\n");
            }else{
                mq_send(clients_messqueue_id[id], (char*)&msg, sizeof(msg), COMM_2ONE); 
            }

            break;
        case COMM_STOP:
            mq_close(clients_messqueue_id[rcv.client_id]);
            clients_messqueue_id[rcv.client_id] = -1;
            active_clients--;
            printf("disconnected client %d\n", rcv.client_id);
            break;
        case COMM_LIST:
            msg.ntype = COMM_LIST;
            msg.client_id = -1;
            char* list_of_clients = list_clients(clients_messqueue_id, rcv.client_id);
            strcpy(msg.mtext, list_of_clients);
            mq_send(clients_messqueue_id[rcv.client_id], (char*)&msg, sizeof(msg), COMM_LIST);

            free(list_of_clients);
            break;
        default:
            printf("invalid message\n");
            break;
    }
}

void clear_mtext(struct msgbuf* msg){
    memset(msg->mtext, 0, MAX_MESSAGE_LENGTH+1);
}

void init_clients_messqueue_id_array(){
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        clients_messqueue_id[i] = -1;
    }
}

void handle_sigint(){
    running = 0;
}