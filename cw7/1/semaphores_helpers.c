#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

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
    int sem_key;
    int mem_key;
} sh_config;

union semun {
int val;
struct semid_ds *buf;
unsigned short  *array;
} arg;

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

sh_config* server_init(){
    char *shm;
    int *s;

    key_t key = ftok("./semafory", 'S');

    int sem_id = semget(key, SEMAPHORE_NUM, IPC_CREAT | 0666);

    sh_config* conf = malloc(sizeof(sh_config));
    conf->sem_key = sem_id;

    arg.val = 1;
    semctl(sem_id, QUEUE_ACCESS, SETVAL, arg);
    
    arg.val = 0;
    semctl(sem_id, WAITINGROOM, SETVAL, arg);
    semctl(sem_id, SEATED, SETVAL, arg);

    arg.val = SEATS_NUM;
    semctl(sem_id, SEATS, SETVAL, arg);

    key = ftok("./sharedmem", 'M');
    int shm_id = shmget(key, sizeof(shared_memory_t), IPC_CREAT | 0666);
    if (shm_id < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    conf->mem_key = shm_id;

    if ((shm = shmat(shm_id, NULL, 0)) == (char*) -1) {
        perror("shmat");
        exit(1);
    }

    s = (int*) shm;

    for (size_t i = 0; i < WAITINGROOM_NUM; i++)
    {
        *s++ = -1;
    }

    *s++ = 0;
    *s = 0;

    shmdt(shm);
    shmdt(s);
    return conf;
}


sh_config* get_stup(){
    key_t key = ftok("./semafory", 'S');

    int sem_id = semget(key, SEMAPHORE_NUM, 0666);

    sh_config* conf = malloc(sizeof(sh_config));
    conf->sem_key = sem_id;

    key = ftok("./sharedmem", 'M');
    int shm_id = shmget(key, sizeof(shared_memory_t), IPC_CREAT | 0666);
    if (shm_id < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    conf->mem_key = shm_id;

    return conf;
}

int reserve(struct sh_config* config, int sem_num){
    struct sembuf message;
    message.sem_num = sem_num;
    message.sem_op = -1;
    message.sem_flg = 0; 

    semop(config->sem_key, &message, 1);
}

int release(struct sh_config* config, int sem_num){
    struct sembuf message;
    message.sem_num = sem_num;
    message.sem_op = 1;
    message.sem_flg = IPC_NOWAIT; 

    semop(config->sem_key, &message, 1);
}

void close_all(struct sh_config* config){
    semctl(config->sem_key, 0, IPC_RMID, arg);
    shmctl(config->mem_key, IPC_RMID, NULL);
}