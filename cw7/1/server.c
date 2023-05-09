#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "semaphores_helpers.c"

int pids[HAIDRESSER_NUM + CLIENTS_NUM];
int ind = 0;

void handler(){
    for (size_t i = 0; i < ind; i++)
    {
        kill(pids[i], SIGINT);
    }
    
}

int main(int argc, char* argv[]){
    struct sh_config* config;
    config = server_init();
    signal(SIGINT, handler);
    

    for (size_t i = 0; i < HAIDRESSER_NUM; i++)
    {
        int child = fork();
        if(child == 0){
            execl("hairdresser", "hairdresser", NULL);
            perror("execl error");
            return EXIT_FAILURE;
        }
        pids[ind] = child;
        ind++;
    }


    for (size_t i = 0; i < CLIENTS_NUM; i++)
    {
        int child = fork();
        if(child == 0){
            execl("client", "client", NULL);
            perror("execl error");
            return EXIT_FAILURE;

        }
        pids[ind] = child;
        ind++;
    }


    pause();
    close_all(config);

}
