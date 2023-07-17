#include "config.h"

char* list_clients(int* clients_messqueue_id, int curr_client);
void handle_rcv();
void clear_mtext(struct msgbuf *msg);
void init_clients_messqueue_id_array();
void handle_sigint();
void disconnect_clients();
void log_message(struct msgbuf* mess);

int clients_messqueue_id[MAX_NUM_OF_CLIENTS];
int server_messqueue_id;
int active_clients = 0;
int next_client_id = 0;
int running = 1;

struct msgbuf rcv;

int main(int argc, char const *argv[])
{      
    init_clients_messqueue_id_array();
    signal(SIGINT, handle_sigint);

    key_t key = ftok(getenv("HOME"), SERVER_KEY_ID);
    server_messqueue_id = msgget(key, IPC_CREAT | 0666);
    printf("Starting server with id = %d\n", server_messqueue_id);

    while (running)
    {
        if(msgrcv(server_messqueue_id, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            handle_rcv();
        }
    }

    disconnect_clients();
    if (msgctl(server_messqueue_id, IPC_RMID, NULL) == -1) {
        perror("Error removing message queue");
    }
    return 0;
}

void disconnect_clients(){
    struct msgbuf msg = {1,-1,""};
    for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
    {
        msg.ntype = COMM_STOP;
        if(clients_messqueue_id[i] != -1){
            msgsnd(clients_messqueue_id[i], &msg, sizeof(msg), IPC_NOWAIT);        
        }
    }

    while(active_clients > 0){
        if(msgrcv(server_messqueue_id, &rcv, sizeof(rcv), -NUMBER_OF_TYPES, IPC_NOWAIT ) != -1){
            if(rcv.ntype == COMM_STOP){
                msgctl(clients_messqueue_id[rcv.client_id], IPC_RMID, NULL);
                clients_messqueue_id[rcv.client_id] = -1;
                --active_clients;
                printf("disconnected client %d\n", rcv.client_id);
            }
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

            if(active_clients == MAX_NUM_OF_CLIENTS){
                msg.client_id = -1;
            }else{
                msg.client_id = next_client_id;
                clients_messqueue_id[next_client_id] = rcv.client_id;
                ++next_client_id;
                ++active_clients;

                printf("new connection %d\n", next_client_id);
            }

            msgsnd(rcv.client_id, &msg, sizeof(msg), IPC_NOWAIT);
            break;
        case COMM_2ALL:
            msg.ntype = COMM_2ALL;
            msg.client_id = rcv.client_id;
            strcpy(msg.mtext, rcv.mtext);
            
            for (size_t i = 0; i < MAX_NUM_OF_CLIENTS; i++)
            {
                if(clients_messqueue_id[i] != -1 && i != rcv.client_id){
                    msgsnd(clients_messqueue_id[i], &msg, sizeof(msg), IPC_NOWAIT);
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
                msgsnd(clients_messqueue_id[id], &msg, sizeof(msg), IPC_NOWAIT);
            }

            break;
        case COMM_STOP:
            msgctl(clients_messqueue_id[rcv.client_id], IPC_RMID, NULL);
            clients_messqueue_id[rcv.client_id] = -1;
            active_clients--;
            printf("disconnected client %d\n", rcv.client_id);
            break;
        case COMM_LIST:
            msg.ntype = COMM_LIST;
            msg.client_id = -1;
            char* list_of_clients = list_clients(clients_messqueue_id, rcv.client_id);
            strcpy(msg.mtext, list_of_clients);
            msgsnd(clients_messqueue_id[rcv.client_id], &msg, sizeof(msg), IPC_NOWAIT);

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