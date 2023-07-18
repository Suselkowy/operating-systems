#include "semaphores_helpers.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
} arg;

sh_config* server_init(){
    int tab[4] = {1, 0, SEATS_NUM, 0};
    char buff[128];
    char *shm;
    int *s;

    sh_config* conf = malloc(sizeof(sh_config));

    for (size_t i = 0; i < SEMAPHORE_NUM; i++)
    {   
        sprintf(buff, "s%d", i);

        conf->sem_key[i] = sem_open(buff, O_CREAT, 0666, tab[i]);

        int val;
        sem_getvalue(conf->sem_key[i], &val);

        if(conf->sem_key[i] == SEM_FAILED){
        perror("sem failed\n");
        exit(EXIT_FAILURE);
        }
    }

    int fd =  shm_open("sharedmemPOSIX", O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        printf("*** shm error (server) ***\n");
        perror("");
        exit(1);
    }
    int check = ftruncate(fd, sizeof(shared_memory_t));
    if(check == -1){
                printf("*** shm error check (server) ***\n");
        exit(1);
    }

    conf->mem_key = fd;

    if ((shm = mmap(NULL, sizeof(shared_memory_t), PROT_READ|PROT_WRITE , MAP_SHARED, fd, 0)) == (char*) -1) {
        perror("mmap\n");
        exit(1);
    }

    s = (int*) shm;

    for (size_t i = 0; i < WAITINGROOM_NUM; i++)
    {
        *s++ = -1;
    }

    *s++ = 0;
    *s = 0;

    munmap(shm, sizeof(shared_memory_t));
    munmap(s, sizeof(int));
    return conf;
}


sh_config* get_client_config(){
    int tab[4] = {1, 0, SEATS_NUM, 0};
    char buff[128];
    char *shm;
    int *s;

    sh_config* conf = malloc(sizeof(sh_config));

    for (size_t i = 0; i < SEMAPHORE_NUM; i++)
    {   
        sprintf(buff, "s%d", i);
        conf->sem_key[i] = sem_open(buff, O_RDWR);

        int val;
        sem_getvalue(conf->sem_key[i], &val);

        if(conf->sem_key[i] == SEM_FAILED){
        perror("sem failed\n");
        exit(EXIT_FAILURE);
        }
    }

    int fd =  shm_open("sharedmemPOSIX", O_RDWR, 0666);
    if (fd < 0) {
        printf("*** shm error ***\n");
        exit(1);
    }

    conf->mem_key = fd;

    return conf;
}

int reserve(struct sh_config* config, int sem_num){
    sem_wait(config->sem_key[sem_num]);
}

int release(struct sh_config* config, int sem_num){
    sem_post(config->sem_key[sem_num]);
}

void close_all(struct sh_config* config){
    char buff[128];
    for (size_t i = 0; i < SEMAPHORE_NUM; i++)
    {
        sprintf(buff, "s%d", i);
        sem_close(config->sem_key[i]);
        sem_unlink(buff);
    }
    
    shm_unlink("sharedmemPOSIX");
}

void detach_shared_memeory(shared_memory_t* shared_memory_pt){
    munmap(shared_memory_pt, sizeof(shared_memory_t));
}

void close_semaphores(struct sh_config* config){
    for (size_t i = 0; i < SEMAPHORE_NUM; i++)
    {
        sem_close(config->sem_key[i]);
    }
}