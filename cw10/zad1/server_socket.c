#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "helper.h"
#define MAX_CLIENTS 10
#define MAX_EVENTS 10

int running = 1;
struct client clients[MAX_CLIENTS];
char server_nickname[] = "Server";
pthread_t ping_thread;

int online_fd;
int local_fd;
int epoll_fd;

void stop_running();
void* ping_service(void* arg);
char* list(int fd);
void close_client(int fd, int* num_of_clients);
int new_client(int* num_of_clients, int* curr_c, int fd);
void init_clients_table();
void init_online_conenction();
void init_local_conenction(char* socket_path);
void add_entry_to_epoll(int epoll_fd, int entry_fd);
void handle_client_message(int client_fd, int* num_of_clients, struct message* rcv_buff, struct message* snd_buff);

int main(int argc, char** argv) {
    if (argc != 3) {
        perror("Invalid number of arguments");
        exit(EXIT_FAILURE);
    }
    init_clients_table();
    signal(SIGINT, stop_running);

    int next_client_id = 0;
    int num_of_clients = 0;
    struct message rcv_buff;
    struct message snd_buff;
    int port = atoi(argv[1]);
    char* socket_path = argv[2];

    init_online_conenction();
    init_local_conenction(socket_path);

    epoll_fd = epoll_create1(0);
    add_entry_to_epoll(epoll_fd, local_fd);
    add_entry_to_epoll(epoll_fd, online_fd);

    printf("Server listening on *:%d and '%s'\n", port, socket_path);

    int result = pthread_create(&ping_thread, NULL, ping_service, (void *)clients);
    if (result != 0) {
        fprintf(stderr, "Error creating ping thread: %d\n", result);
        exit(EXIT_FAILURE);
    }

    int number_of_events;
    struct epoll_event events[MAX_EVENTS];
    while(running){
        number_of_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (number_of_events == -1) {
            perror("Error waiting for events.\n");
            return -1;
        }

        for (size_t i = 0; i < number_of_events; i++)
        {
            int fd = events[i].data.fd;
            if(fd == local_fd){
                printf("local connection\n");
                new_client(&num_of_clients, &next_client_id, local_fd);
            }else if(fd == online_fd){
                printf("online connection\n");
                new_client(&num_of_clients, &next_client_id, online_fd);
            }else{
                if(read(fd, &rcv_buff, sizeof(struct message)) == -1){
                    perror("Error reciving message from client in server\n");
                }else{
                    handle_client_message(fd, &num_of_clients, &rcv_buff, &snd_buff);
                }
                
            }
        }
    
    }
    printf("Server shutdown\n");
}

void handle_client_message(int client_fd, int* num_of_clients, struct message* rcv_buff, struct message* snd_buff){
    int type = rcv_buff->type;
    switch(type){
    case COMM_INIT:
        int nick_taken = 0;
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].fd != -1){
                if(strlen(clients[i].nickname) == strlen(rcv_buff->from) && strncmp(clients[i].nickname, rcv_buff->from, strlen(rcv_buff->from))==0){
                    nick_taken = 1;
                    break;
                }
            }
        }
        if(nick_taken){
            snd_buff->type = COMM_INIT;
            memcpy(snd_buff->to, rcv_buff->from, strlen(rcv_buff->from)+1);
            close_client(client_fd, num_of_clients);
        }else{
            snd_buff->type = COMM_INIT;
            memcpy(snd_buff->to, rcv_buff->from, strlen(rcv_buff->from)+1);
            for (size_t i = 0; i < MAX_CLIENTS; i++)
            {
                if(clients[i].fd == client_fd){
                    strcpy(clients[i].nickname, rcv_buff->from);
                    break;
                }
            }
            
            if(write(client_fd, snd_buff, sizeof(struct message)) == -1){
                perror("Error sending message to client\n");
            }
            printf("Init succesfull for %s\n\n", rcv_buff->from);
        }
        break;
    case COMM_STOP:
        snd_buff->type = COMM_STOP;
        printf("closed\n");
        close_client(client_fd, num_of_clients);
        break;
    case COMM_2ALL:
        printf("Message from %s to all. Body: %s\n\n", rcv_buff->from, rcv_buff->message);
        snd_buff->type = COMM_2ALL;
        strcpy(snd_buff->message, rcv_buff->message);
        strcpy(snd_buff->from, rcv_buff->from);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].fd != -1 && client_fd != clients[i].fd){
                    if(write(clients[i].fd , snd_buff, sizeof(struct message)) == -1){
                        perror("Error sending message to client\n");
                    }  
            }
        }
        break;
    case COMM_2ONE:
        printf("Message from %s to %s. Body: %s\n\n", rcv_buff->from, rcv_buff->to ,rcv_buff->message);
        snd_buff->type = COMM_2ONE;
        strcpy(snd_buff->message, rcv_buff->message);
        strcpy(snd_buff->from, rcv_buff->from);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].fd != -1){
                if(strcmp(clients[i].nickname, rcv_buff->to)==0){
                    strcpy(snd_buff->to, clients[i].nickname);
                    if(write(clients[i].fd , snd_buff, sizeof(struct message)) == -1){
                        perror("Error sending message to client\n");
                    }
                    break;
                }
            }
        }
        break;
    case COMM_LIST:
        printf("List message\n\n");
        char* list_mess = list(client_fd);
        snd_buff->type = COMM_LIST;
        memcpy(snd_buff->to, rcv_buff->from, strlen(rcv_buff->from)+1);
        memcpy(snd_buff->from, server_nickname, strlen(server_nickname)+1);
        strcpy(snd_buff->message, list_mess);
        if(write(client_fd, snd_buff, sizeof(struct message)) == -1){
            perror("Error sending message to client\n");
        }
        break;
    case COMM_PING:
        if(strcmp(rcv_buff->from, server_nickname) == 0){
            close_client(client_fd, num_of_clients);
        }
    }
}

void add_entry_to_epoll(int epoll_fd, int entry_fd){
    struct epoll_event ev;
    ev.data.fd = entry_fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, entry_fd, &ev) < 0)
    {
        fprintf(stderr, "Failed to add socket to event pool.\n");
        exit(1);
    }
}

void init_clients_table(){
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].fd = -1;
    }
}

void init_online_conenction(){
    if((online_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("blad socket online\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in online_addr;
    online_addr.sin_family = AF_INET;
    online_addr.sin_port =  htons(12345);
    online_addr.sin_zero[0] = '\0';
    online_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    setsockopt(online_fd, SOL_SOCKET, SO_REUSEADDR, &online_addr, sizeof(online_addr));
    if(bind(online_fd, (struct sockaddr*)&online_addr, sizeof(online_addr)) == -1){
        perror("Error binding online\n");
    }
    if(listen(online_fd, MAX_CLIENTS) == -1){
        perror("Error listen\n");
    }
}

void init_local_conenction(char* socket_path){
    if((local_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        printf("blad socket local\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un local_addr;
    local_addr.sun_family = AF_UNIX;
    strncpy(local_addr.sun_path, socket_path, sizeof(local_addr.sun_path));
    setsockopt(local_fd, SOL_SOCKET, SO_REUSEADDR, &local_addr, sizeof(local_addr));
    unlink(socket_path);
    if(bind(local_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1){
        perror("Error binding local\n");
    }
    if( listen(local_fd, MAX_CLIENTS) == -1){
        perror("Error listen\n");
    }
}

void stop_running(){
    running = 0;
    pthread_cancel(ping_thread);
    pthread_join(ping_thread, NULL);
    struct message snd_buff;
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].fd != -1){
            snd_buff.type = COMM_STOP;
            if(write(clients[i].fd, &snd_buff, sizeof(struct message)) == -1){
                perror("Error sending message to client\n");
            }
            close(clients[i].fd);
        }
    }
    printf("%d %d\n", online_fd, local_fd);
    shutdown(online_fd, SHUT_RDWR);
    shutdown(local_fd, SHUT_RDWR);
    close(epoll_fd);
    close(online_fd);
    close(local_fd);
    printf("stop running\n");
    exit(EXIT_SUCCESS);
}

void* ping_service(void* arg){
    struct message snd_buff;
    struct client* clients = (struct client*) arg;
    while(1){
        sleep(4);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].fd != -1){
                snd_buff.type = COMM_PING;
                if(write(clients[i].fd, &snd_buff, sizeof(struct message)) == -1){
                    perror("Error sending message to client\n");
                }
            }
        }
    }
    return arg;
}

char* list(int fd){
    char* ans = calloc((MAX_NICK_LENGHT+2)*MAX_CLIENTS + 1, sizeof(char));
    int j = 0;
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].fd != -1 && clients[i].fd != fd){
            sprintf(&ans[j], "%s, ", clients[i].nickname);
            j+= strlen(&ans[j]);
        }
    }
    return ans;
}

void close_client(int fd, int* num_of_clients){
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].fd == fd){
            clients[i].fd = -1;
            break;
        }
    }
    *num_of_clients -= 1;
    close(fd);
}

int new_client(int* num_of_clients, int* curr_c, int fd){
    int client_desc = -1;
    struct sockaddr client;
    socklen_t addrlen;
    int curr_client = *curr_c;
    if((client_desc = accept(fd, &client, &addrlen)) != -1){
        printf("new client\n");
        if(*num_of_clients >= MAX_CLIENTS){
            close_client(client_desc, num_of_clients);
            return client_desc;
        }

        if(clients[curr_client].fd != -1){
            while(clients[curr_client].fd != -1){
                curr_client += 1;
                if(curr_client >= MAX_CLIENTS){
                    curr_client = 0;
                }
            }
        }
        clients[curr_client].fd = client_desc;
        *curr_c = curr_client+1;
        *num_of_clients += 1;

        struct epoll_event event;
        event.data.fd = client_desc;
        event.events = EPOLLIN;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_desc, &event) < 0)
        {
            fprintf(stderr, "Failed to add client socket to event pool.\n");
        }
    }else{
        printf("error in new client\n");
    }
}