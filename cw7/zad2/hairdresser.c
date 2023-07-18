#include "semaphores_helpers.h"

struct sh_config* config;
struct shared_memory_t* shm_ptr;

void handle_sigint();

int main(int argc, char* argv[]){
    signal(SIGINT, handle_sigint);

    config = get_client_config();

    shm_ptr = (shared_memory_t *) mmap(NULL, sizeof(shared_memory_t), PROT_READ|PROT_WRITE , MAP_SHARED, config->mem_key, 0);

    while(1){
        printf("Waiting for clients\n");
        reserve(config, WAITINGROOM);

        printf("Attend to client\n");
        reserve(config, QUEUE_ACCESS);
        int hairstyle = shm_ptr->queue[shm_ptr->queue_start];
        int tmp = shm_ptr->queue_start;
        shm_ptr->queue[shm_ptr->queue_start] = -1;
        shm_ptr->queue_start = (tmp + 1) % WAITINGROOM_NUM;
        release(config, QUEUE_ACCESS);

        reserve(config, SEATS);
        release(config, SEATED);
        printf("Hairdresser is working %d\n", hairstyle);
        sleep(hairstyle);
        printf("Hairdresser ended working on %d\n", hairstyle);
        release(config, SEATS);
    }
    
    return 0;
}

void handle_sigint(){
    detach_shared_memeory(shm_ptr);
    close_semaphores(config);
    exit(EXIT_FAILURE);
}