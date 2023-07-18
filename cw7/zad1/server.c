#include "semaphores_helpers.h"

int processes_id[HAIDRESSER_NUM + CLIENTS_NUM];
int next_process_id = 0;

void handle_sigint();
void create_hairdressers();
void create_clients();

int main(int argc, char* argv[]){
    struct sh_config* config;
    config = server_init();
    signal(SIGINT, handle_sigint);
    
    create_hairdressers();

    create_clients();

    pause();
    close_all(config);
}

void handle_sigint(){
    for (size_t i = 0; i < next_process_id; i++)
    {
        kill(processes_id[i], SIGINT);
    }
    
}

void create_hairdressers(){
    for (size_t i = 0; i < HAIDRESSER_NUM; i++)
    {
        int child = fork();
        if(child == 0){
            execl("hairdresser", "hairdresser", NULL);
            perror("execl error");
            exit(EXIT_FAILURE);
        }
        processes_id[next_process_id] = child;
        next_process_id++;
    }
}

void create_clients(){
    for (size_t i = 0; i < CLIENTS_NUM; i++)
    {
        int child = fork();
        if(child == 0){
            execl("client", "client", NULL);
            perror("execl error");
            exit(EXIT_FAILURE);
        }
        processes_id[next_process_id] = child;
        next_process_id++;
    }
}