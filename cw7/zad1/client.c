#include "semaphores_helpers.h"
#include <time.h>

struct sh_config* config;
struct shared_memory_t* shm_ptr;
void handle_sigint();
void add_client_to_waitingroom(int hairstyle);

int main(int argc, char* argv[]){
    signal(SIGINT, handle_sigint);
    srand(time(NULL) + getpid());
    
    config = get_client_config();
    shm_ptr = (shared_memory_t *) shmat(config->mem_key, NULL, 0);

    int hairstyle = (rand() % STYLE_NUM) + 2;
    add_client_to_waitingroom(hairstyle);

    reserve(config, SEATED);
    printf("Client on seat with hairstyle: %d\n", hairstyle);
    sleep(hairstyle);
    printf("Client with hairstyle: %d, finished\n", hairstyle);
    return 0;
}

void handle_sigint(){
    detach_shared_memeory(shm_ptr);
    exit(EXIT_FAILURE);
}

void add_client_to_waitingroom(int hairstyle){
    reserve(config, QUEUE_ACCESS);
    int next_id = shm_ptr->queue_tail+1 % WAITINGROOM_NUM;
    if(shm_ptr->queue[shm_ptr->queue_tail] != -1){
        fprintf(stderr, "Waiting room full\n");
        release(config, QUEUE_ACCESS);
        raise(SIGINT);
        return;
    }
    shm_ptr->queue[shm_ptr->queue_tail] = hairstyle;
    shm_ptr->queue_tail = next_id;
    printf("Client in waitingroom\n");
    release(config, WAITINGROOM);
    release(config, QUEUE_ACCESS);
}