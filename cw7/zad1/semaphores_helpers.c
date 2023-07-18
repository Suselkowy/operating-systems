#include "semaphores_helpers.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
} arg;

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


sh_config* get_client_config(){
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

void detach_shared_memeory(shared_memory_t* shared_memory_pt){
    shmdt(shared_memory_pt);
}
