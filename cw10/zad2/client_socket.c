#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "helper.h"
#include <string.h>
#include <sys/epoll.h>
#include <signal.h>

int fd = -1;
int running = 1;

void stop_running(){
    close(fd);
    printf("stop running\n");
    exit(EXIT_SUCCESS);
}

void signal_handler(){
    struct message snd_buff;
    snd_buff.type = COMM_STOP;
    if(write(fd, &snd_buff, sizeof(struct message)) == -1){
        perror("Error sending init message\n");
        exit(EXIT_FAILURE);
    }
    stop_running();
}

int main(int argc, char const *argv[])
{   
    if (argc != 5 && argc != 4) {
        printf("Usage: %s <client_name> network <server_address> <port> or %s <client_name> local <path>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* client_name = argv[1];
    const char* connection_type = argv[2];
    const char* server_address = argv[3];
    int port = -1;
    if (argc == 5) {
        port = atoi(argv[4]);
    }

    if(strlen(client_name) > MAX_NICK_LENGHT){
        printf("Nick too long\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, signal_handler);

    if(strcmp(connection_type, "network") == 0){
        if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            printf("Error creating socket\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port =  htons(port);
        addr.sin_zero[0] = '\0';
        
        if (inet_pton(AF_INET, server_address, &addr.sin_addr) <= 0) {
            perror("Invalid server address.\n");
            return -1;
        }

        if(connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1){
            perror("Error connecting");
        }
    }    else if (strcmp(connection_type, "local") == 0) {
        if((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
            printf("Error creating socket\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un addr;
        struct sockaddr_un bind_addr;
        addr.sun_family = AF_UNIX;
        bind_addr.sun_family = AF_UNIX;

        strncpy(addr.sun_path, server_address, sizeof(addr.sun_path));
        snprintf(bind_addr.sun_path, sizeof bind_addr.sun_path, "/tmp/%s", client_name);

        unlink(bind_addr.sun_path);
        if (bind(fd, (void*) (struct sockaddr*)&bind_addr, sizeof addr) < 0) {
            perror("Error connecting to server.\n");
            return -1;
        }

        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("Error connecting to server.\n");
            return -1;
        }
    }
    else {
        printf("Invalid connection type: '%s'.\n", connection_type);
        return -1;
    }

    struct message snd_buff;
    struct message rcv_buff;

    snd_buff.type = COMM_INIT;
    memcpy(snd_buff.from, client_name, strlen(client_name)+1);
    printf("nick : %s\n", snd_buff.from);
    if(write(fd, &snd_buff, sizeof(struct message)) == -1){
        perror("Error sending init message\n");
        exit(EXIT_FAILURE);
    }


    int epoll_fd = epoll_create1(0);

    struct epoll_event ev;

    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0)
    {
        fprintf(stderr, "Failed to add local socket to event pool.\n");
        exit(1);
    }

    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        fprintf(stderr, "Failed to add local socket to event pool.\n");
        exit(1);
    }

    struct epoll_event events[2];
    int nfds;
    char* buff = NULL;
	size_t buff_size;
    while(running){
        nfds = epoll_wait(epoll_fd, events, 2, -1);
        if (nfds == -1) {
            perror("Error waiting for events.\n");
            return -1;
        }
        for (size_t i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == STDIN_FILENO) {
		        getline(&buff, &buff_size, stdin);
                if(buff_size > 0){
                    if(strncmp(buff, "LIST", 4) == 0){
                        snd_buff.type = COMM_LIST;
                        if(write(fd, &snd_buff, sizeof(struct message)) == -1){
                            perror("Error sending init message\n");
                            exit(EXIT_FAILURE);
                        }
                    }else if(strncmp(buff, "2ALL", 4) == 0){
                        snd_buff.type = COMM_2ALL;
                        if(strlen(&buff[5]) > MAX_MESSAGE_LENGTH-1){
                            strncpy(snd_buff.message, '\0', sizeof('\0'));
                            strncat(snd_buff.message, &buff[5], MAX_MESSAGE_LENGTH-1);
                        }else{
                            strcpy(snd_buff.message, &buff[5]);
                        }

                        if(write(fd, &snd_buff, sizeof(struct message)) == -1){
                            perror("Error sending init message\n");
                            exit(EXIT_FAILURE);
                        }
                    }else if(strncmp(buff, "STOP", 4) == 0){
                        printf("closed\n");
                        raise(SIGINT);
                    }else if(strncmp(buff, "2ONE", 4) == 0){
                        char* to = strtok(&buff[5], " ");
                        char* mess = strtok(NULL, "\n");
                        if(strlen(mess) > MAX_MESSAGE_LENGTH-1){
                            strncpy(snd_buff.message, '\0', sizeof('\0'));
                            strncat(snd_buff.message, mess, MAX_MESSAGE_LENGTH-1);
                        }else{
                            strcpy(snd_buff.message, mess);
                        }

                        if(strlen(to) > MAX_NICK_LENGHT-1){
                            printf("Message not sent, nick too long\n");
                            continue;
                        }
                        strcpy(snd_buff.to, to);
                        snd_buff.type = COMM_2ONE;
                        if(write(fd, &snd_buff, sizeof(struct message)) == -1){
                            perror("Error sending init message\n");
                            exit(EXIT_FAILURE);
                        }
                    }else{
                        printf("Invalid command\n");
                    }
                }
            }else{
                if(read(fd, &rcv_buff, sizeof(struct message)) == -1){
                    perror("Error reciving init message\n");
                    exit(EXIT_FAILURE);
                }
                if(rcv_buff.type == COMM_INIT){
                    if(strncmp(rcv_buff.to, client_name, strlen(client_name))==0){
                        printf("Connected to server with nick: %s\n", client_name);
                    }else{
                        printf("Connection to server refused: nick '%s' already in use. Try again with difrent one.\n");
                        shutdown(fd, SHUT_RDWR);
                        close(fd);
                        exit(EXIT_SUCCESS);
                    }
                }
                else if(rcv_buff.type == COMM_STOP){
                    raise(SIGINT);
                }else if(rcv_buff.type == COMM_PING){
                    //printf("ping\n");
                        snd_buff.type = COMM_PING;
                        if(write(fd, &snd_buff, sizeof(struct message)) == -1){
                            perror("Error sending init message\n");
                            exit(EXIT_FAILURE);
                        }
                }
                else{
                    if(rcv_buff.type == COMM_2ALL){
                        printf("Broadcast message from %s : %s", rcv_buff.from ,rcv_buff.message);
                    }else if(rcv_buff.type == COMM_2ONE){
                        printf("Private message from %s: %s\n", rcv_buff.from ,rcv_buff.message);
                    }else if(rcv_buff.type == COMM_LIST){
                        printf("List of users: %s\n", rcv_buff.message);
                    }else{
                        printf("Invalid message recived\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        
    }




    return 0;
}
