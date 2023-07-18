#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define HAIDRESSER_NUM 4
#define SEATS_NUM 3
#define WAITINGROOM_NUM 5
#define SEMAPHORE_NUM 4
#define CLIENTS_NUM 10

typedef struct shared_memory_t
{
    int queue[WAITINGROOM_NUM];
    int queue_start;
    int queue_tail;
} shared_memory_t;

typedef struct sh_config{
    sem_t* sem_key[SEMAPHORE_NUM];
    int mem_key;
} sh_config;

enum hairstyle{
    STYLE_SHORT,
    STYLE_LONG,
    STYLE_SPIKY,
    STYLE_BALD,
    STYLE_NUM
};

enum semaphore{
    QUEUE_ACCESS,
    WAITINGROOM,
    SEATS,
    SEATED
};

sh_config* server_init();
sh_config* get_client_config();
int reserve(struct sh_config* config, int sem_num);
int release(struct sh_config* config, int sem_num);
void close_all(struct sh_config* config);
void detach_shared_memeory(shared_memory_t* shared_memory_pt);
void close_semaphores(struct sh_config* config);