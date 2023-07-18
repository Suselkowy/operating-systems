#include <netinet/in.h>
#include "helper.h"

int socket_fd;
int running = 1;
struct message snd_buff;
struct message rcv_buff;

void stop_running();
void handle_sigint();
void handle_socket(const char* client_name);
void handle_stdin();
void add_entry_to_epoll(int epoll_fd, int entry_fd);
void send_init_message(const char* client_name);
void connect_to_server(const char* connection_type, const char* server_address, int port);

int main(int argc, char const *argv[])
{   
    if (argc != 5 && argc != 4) {
        printf("Usage: %s <client_name> network <server_address> <port> or %s <client_name> local <path>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);

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

    connect_to_server(connection_type, server_address, port);
    send_init_message(client_name);

    struct message snd_buff;
    struct message rcv_buff;

    int epoll_fd = epoll_create1(0);
    add_entry_to_epoll(epoll_fd, STDIN_FILENO);
    add_entry_to_epoll(epoll_fd, socket_fd);

    struct epoll_event events[2];
    int number_of_fd;
    while(running){
        number_of_fd = epoll_wait(epoll_fd, events, 2, -1);

        if (number_of_fd == -1) {
            perror("Error waiting for events.\n");
            return -1;
        }

        for (size_t i = 0; i < number_of_fd; i++)
        {
            if (events[i].data.fd == STDIN_FILENO) {
		        handle_stdin();
            }else{
                handle_socket(client_name);
            }
        }
        
    }

    return 0;
}

void connect_to_server(const char* connection_type, const char* server_address, int port){
    if(strcmp(connection_type, "network") == 0){
        if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            printf("Error creating socket\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port =  htons(port);
        addr.sin_zero[0] = '\0';
        
        if (inet_pton(AF_INET, server_address, &addr.sin_addr) <= 0) {
            perror("Invalid server address.\n");
            exit(EXIT_FAILURE);
        }

        if(connect(socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1){
            perror("Error connecting");
            exit(EXIT_FAILURE);
        }
    }    else if (strcmp(connection_type, "local") == 0) {
        if((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
            printf("Error creating socket\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, server_address, sizeof(addr.sun_path));

        if (connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("Error connecting to server.\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        printf("Invalid connection type: '%s'.\n", connection_type);
        exit(EXIT_FAILURE);
    }
}

void send_init_message(const char* client_name){
    snd_buff.type = COMM_INIT;
    memcpy(snd_buff.from, client_name, strlen(client_name)+1);
    printf("nick : %s\n", snd_buff.from);
    if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
        perror("Error sending init message\n");
        exit(EXIT_FAILURE);
    }
}

void add_entry_to_epoll(int epoll_fd, int entry_fd){
    struct epoll_event ev;
    ev.data.fd = entry_fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, entry_fd, &ev) < 0)
    {
        fprintf(stderr, "Failed to add socket to event pool\n");
        exit(EXIT_FAILURE);
    }
}

void handle_stdin(){
    char* buff = NULL;
	size_t buff_size;

    getline(&buff, &buff_size, stdin);
    if(buff_size > 0){
        if(strncmp(buff, "LIST", 4) == 0){
            snd_buff.type = COMM_LIST;
            if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
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

            if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
                perror("Error sending init message\n");
                exit(EXIT_FAILURE);
            }
        }else if(strncmp(buff, "STOP", 4) == 0){
            printf("closed\n");
            raise(SIGINT);
        }else if(strncmp(buff, "2ONE", 4) == 0){
            if(buff_size < 5){
                printf("Message invalid: command to short\n");
                return;
            }
            char* to = strtok(&buff[5], " ");
            if(to == NULL){
                printf("Message invalid: no recipient provied\n");
                perror("");
            }
            char* mess = strtok(NULL, "\n");
            if(mess == NULL){
                mess = " ";
            }
            if(strlen(mess) > MAX_MESSAGE_LENGTH-1){
                strncpy(snd_buff.message, '\0', sizeof('\0'));
                strncat(snd_buff.message, mess, MAX_MESSAGE_LENGTH-1);
            }else{
                strcpy(snd_buff.message, mess);
            }

            if(strlen(to) > MAX_NICK_LENGHT-1){
                printf("Message not sent, nick too long\n");
                return;
            }
            strcpy(snd_buff.to, to);
            snd_buff.type = COMM_2ONE;
            if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
                perror("Error sending init message\n");
                exit(EXIT_FAILURE);
            }
        }else{
            printf("Invalid command\n");
        }
    }
    free(buff);
}

void handle_socket(const char* client_name){
    if(read(socket_fd, &rcv_buff, sizeof(struct message)) == -1){
        perror("Error reciving init message\n");
        exit(EXIT_FAILURE);
    }
    if(rcv_buff.type == COMM_INIT){
        if(strncmp(rcv_buff.to, client_name, strlen(client_name))==0){
            printf("Connected to server with nick: %s\n", client_name);
        }else{
            printf("Connection to server refused: nick '%s' already in use. Try again with difrent one.\n");
            shutdown(socket_fd, SHUT_RDWR);
            close(socket_fd);
            exit(EXIT_SUCCESS);
        }
    }
    else if(rcv_buff.type == COMM_STOP){
        raise(SIGINT);
    }else if(rcv_buff.type == COMM_PING){
        snd_buff.type = COMM_PING;
        if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
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

void stop_running(){
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    printf("stop running\n");
    exit(EXIT_SUCCESS);
}

void handle_sigint(){
    struct message snd_buff;
    snd_buff.type = COMM_STOP;
    if(write(socket_fd, &snd_buff, sizeof(struct message)) == -1){
        perror("Error sending init message\n");
        exit(EXIT_FAILURE);
    }
    stop_running();
}