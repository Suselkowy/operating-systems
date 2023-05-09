#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include "semaphores_helpers.c"
#include <time.h>
#include <signal.h>
struct sh_config* config;

void handler(){
    close_all(config);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]){
    signal(SIGINT, handler);
    srand(time(NULL) + getpid());
    
    config = get_stup();

    struct shared_memory_t* shm_ptr = (shared_memory_t *) mmap(NULL, sizeof(shared_memory_t), PROT_READ|PROT_WRITE , MAP_SHARED, config->mem_key, 0);
    if ((char* )shm_ptr == (char*) -1) {
        perror("mmap\n");
        exit(1);
    }
    
    reserve(config, QUEUE_ACCESS);

    int next_id = shm_ptr->queue_tail+1 % WAITINGROOM_NUM;
    if(shm_ptr->queue[shm_ptr->queue_tail] != -1){
        fprintf(stderr, "Waiting room full\n");
        release(config, QUEUE_ACCESS);
        return EXIT_FAILURE;
    }

    int hairstyle = (rand() % STYLE_NUM) + 2;
    shm_ptr->queue[shm_ptr->queue_tail] = hairstyle;
    shm_ptr->queue_tail = next_id;
    printf("Client in waitingroom\n");
    release(config, WAITINGROOM);
    release(config, QUEUE_ACCESS);
    reserve(config, SEATED);
    printf("Client on seat with hairstyle: %d\n", hairstyle);

    sleep(hairstyle);

    printf("Client with hairstyle: %d, finished\n", hairstyle);

    return 0;
}