#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>

#include <sys/epoll.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
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

void close_client(union addr* addr, int addr_len, int* num_of_clients){
    struct message snd_buff;
    snd_buff.type = COMM_STOP;
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if( memcmp(&clients[i].addr, addr, addr_len) == 0 ){
            clients[i].fd = -1;
            sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) addr, addr_len);
            printf("client closed %s tu\n", clients[i].nickname);
            break;
        }
    }
    *num_of_clients -= 1;
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
            if(sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &clients[i].addr, clients[i].addr_len)){
                perror("Error sending message to client\n");
            }
        }
    }
    close(epoll_fd);
    close(online_fd);
    close(local_fd);
    printf("stop running\n");
    exit(EXIT_SUCCESS);
}

void* ping_service(void* arg){
    struct message snd_buff;
    int* num_of_clients = (int*) arg;
    while(1){
        sleep(4);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].fd != -1){
                //printf("client pinged name %s id: %d\n", clients[i].nickname, i);
                if(clients[i].responding){
                    clients[i].responding = 0;
                    snd_buff.type = COMM_PING;
                        if(sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &clients[i].addr, clients[i].addr_len) == -1){
                            perror("Error sending message to client\n");
                        }
                }else{
                    //printf("client not responding");
                    close_client(&clients[i].addr, clients[i].addr_len, num_of_clients);
                }

            }
        }
        printf("Ping\n\n");
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

int main(int argc, char** argv) {
    if (argc != 3) {
        perror("Invalid number of arguments");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].fd = -1;
        clients[i].responding = 1;
    }
    

    signal(SIGINT, stop_running);

    int curr_client = 0;
    int num_of_clients = 0;

    int port = atoi(argv[1]);
    char* socket_path = argv[2];

    if((online_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("blad socket online\n");
        exit(EXIT_FAILURE);
    }

    
    if((local_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        printf("blad socket local\n");
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


    struct sockaddr_un local_addr;
    local_addr.sun_family = AF_UNIX;
    strncpy(local_addr.sun_path, socket_path, sizeof(local_addr.sun_path));
    setsockopt(local_fd, SOL_SOCKET, SO_REUSEADDR, &local_addr, sizeof(local_addr));
    unlink(socket_path);
    if(bind(local_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1){
        perror("Error binding local\n");
    }

    epoll_fd = epoll_create1(0);

    struct epoll_event ev;
    struct message rcv_buff;
    struct message snd_buff;

    ev.data.fd = local_fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_fd, &ev) < 0)
    {
        fprintf(stderr, "Failed to add local socket to event pool.\n");
        exit(1);
    }

    ev.data.fd = online_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, online_fd, &ev) < 0)
    {
        fprintf(stderr, "Failed to add local socket to event pool.\n");
        exit(1);
    }


    printf("Server listening on *:%d and '%s'\n", port, socket_path);
    int check = pthread_create(&ping_thread, NULL, ping_service, (void *)clients);
    int nfds;
    struct epoll_event events[MAX_EVENTS];
    while(running){
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error waiting for events.\n");
            return -1;
        }
        for (size_t i = 0; i < nfds; i++)
        {   
            union addr addr;
            socklen_t addrlen = sizeof(addr);
            
            int fd = events[i].data.fd;

            if(recvfrom(fd, &rcv_buff, sizeof(struct message), 0, (struct sockaddr*)&addr, &addrlen) == -1){
                    perror("Error reciving message from client in server\n");
                    continue;
            }
            
            //printf("message from client\n type:%d\n", rcv_buff.type);
            int type = rcv_buff.type;
            switch(type){
                case COMM_INIT:
                    int nick_taken = 0;
                    for (size_t i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(clients[i].fd != -1){
                            if(strlen(clients[i].nickname) == strlen(rcv_buff.from) && strncmp(clients[i].nickname, rcv_buff.from, strlen(rcv_buff.from))==0){
                                nick_taken = 1;
                                break;
                            }
                        }
                    }
                    if(nick_taken){
                        num_of_clients += 1;
                        close_client(&addr, addrlen, &num_of_clients);
                    }else{
                        snd_buff.type = COMM_INIT;
                        memcpy(snd_buff.to, rcv_buff.from, strlen(rcv_buff.from)+1);
                        if(num_of_clients >= MAX_CLIENTS){
                            close_client(&addr, addrlen, &num_of_clients);
                        }
                        if(clients[curr_client].fd != -1){
                            while(clients[curr_client].fd != -1){
                                curr_client += 1;
                                if(curr_client >= MAX_CLIENTS){
                                    curr_client = 0;
                                }
                            }
                        }
                        clients[curr_client].fd = 1;
                        memcpy(&clients[curr_client].addr, (struct sockaddr*)&addr, addrlen);
                        clients[curr_client].addr_len = addrlen;
                        clients[curr_client].sock = fd;
                        strcpy(clients[curr_client].nickname, rcv_buff.from);
                        num_of_clients += 1;
                        curr_client += 1;
                        
                        
                        if(sendto(clients[curr_client-1].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &addr, addrlen) == -1){
                            perror("Error sending message to client\n");
                        }else{
                            printf("Init succesfull for %s\n\n", rcv_buff.from);
                        }
                    }
                    break;
                case COMM_STOP:
                    snd_buff.type = COMM_STOP;
                    printf("closed\n");
                    close_client(&addr, addrlen, &num_of_clients);
                    break;
                case COMM_2ALL:
                    printf("Message from %s to all. Body: %s\n\n", rcv_buff.from, rcv_buff.message);
                    snd_buff.type = COMM_2ALL;
                    strcpy(snd_buff.message, rcv_buff.message);
                    strcpy(snd_buff.from, rcv_buff.from);
                    for (size_t i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(clients[i].fd != -1){
                                if(sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &clients[i].addr, clients[i].addr_len) == -1){
                                    perror("Error sending message to client\n");
                                }  
                        }
                    }
                    break;
                case COMM_2ONE:
                    printf("Message from %s to %s. Body: %s\n\n", rcv_buff.from, rcv_buff.to ,rcv_buff.message);
                    snd_buff.type = COMM_2ONE;
                    strcpy(snd_buff.message, rcv_buff.message);
                    strcpy(snd_buff.from, rcv_buff.from);
                    for (size_t i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(clients[i].fd != -1){
                            if(strcmp(clients[i].nickname, rcv_buff.to)==0){
                                strcpy(snd_buff.to, clients[i].nickname);
                                if(sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &clients[i].addr, clients[i].addr_len) == -1){
                                    perror("Error sending message to client\n");
                                }
                                break;
                            }
                        }
                    }
                    break;
                case COMM_LIST:
                    printf("List message\n\n");
                    char* list_mess = list(fd);
                    snd_buff.type = COMM_LIST;
                    memcpy(snd_buff.to, rcv_buff.from, strlen(rcv_buff.from)+1);
                    memcpy(snd_buff.from, server_nickname, strlen(server_nickname)+1);
                    strcpy(snd_buff.message, list_mess);
                    if(sendto(clients[i].sock, &snd_buff, sizeof(struct message), 0, (struct sockaddr*) &addr, addrlen) == -1){
                        perror("Error sending message to client\n");
                    }
                    break;
                case COMM_PING:
                    //printf("Ping message\n");
                    for (size_t i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(strcmp(rcv_buff.from, clients[i].nickname)==0){
                            clients[i].responding = 1;
                        }
                    }
            }
        }
    
    }
    printf("out");
}

