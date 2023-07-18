#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#define _GNU_SOURCE
#include <unistd.h>

#include "helpers.h"

void* thread_routine_santa(void* arg);
void* thread_routine_reindeer(void* arg);
void* thread_routine_elf(void* arg);
void wake_up();
void sig_handler_thread();
void create_santa(pthread_t** thread_ids, struct thread_config* conf);
void create_reindeers(pthread_t** thread_ids, struct thread_config* conf);
void create_elves(pthread_t** thread_ids, struct thread_config* conf);
void cleanup(pthread_t** thread_ids, sem_t* semaphores, size_t semaphores_count,
             int* waiting_elves, int* waiting_reindeers);


int main(int argc, char* argv[])
{   
    signal(SIGUSR1, wake_up);
    pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;
    int mut = pthread_mutex_init(&x_mutex, NULL);

    if(mut != 0){
        perror("Mutex create error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    int con = pthread_cond_init(&cond, NULL);

    if(con != 0){
        perror("Mutex create error!\n");
        exit(EXIT_FAILURE);
    }

    int* waiting_reindeers = malloc(sizeof(int) * 9);
    if(waiting_reindeers == NULL){
        perror("Malloc failed!\n");
        exit(EXIT_FAILURE);
    }
    int reindeers_in_queue = 0;

    int* waiting_elves = malloc(sizeof(int) * 3);
    if(waiting_elves == NULL){
        perror("Malloc failed!\n");
        exit(EXIT_FAILURE);
    }
    int elves_in_queue = 0;

    int semaphores_count = 3;

    sem_t* semaphores = malloc(sizeof(sem_t) * semaphores_count);
    if(semaphores == NULL){
        perror("Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < semaphores_count; i++)
    {
        int tmp = sem_init(&semaphores[i], 0, 0);
        if(tmp != 0){
            perror("Semaphore init failed!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    struct thread_config conf;
    conf.mutex = &x_mutex;
    conf.cond = &cond;
    conf.waiting_elves = waiting_elves;
    conf.elves_in_queue = &elves_in_queue;
    conf.waiting_reindeers = waiting_reindeers;
    conf.reindeers_in_queue = &reindeers_in_queue;
    conf.semaphores = semaphores;
    conf.parent_pid = getpid();

    pthread_t** thread_ids = malloc(sizeof(pthread_t*) * 20);

    create_santa(thread_ids, &conf);
    create_reindeers(thread_ids, &conf);
    create_elves(thread_ids, &conf);
    
    pause();

    cleanup(thread_ids, semaphores, semaphores_count, waiting_elves, waiting_reindeers);
    return 0;   
}

void create_santa(pthread_t** thread_ids, struct thread_config* conf){
    thread_ids[0] = (pthread_t*) malloc(sizeof(pthread_t));
    int test_value = pthread_create(thread_ids[0], NULL, thread_routine_santa, (void *)conf);
    if(test_value != 0){
        perror("Failed to start thread!\n");
        exit(EXIT_FAILURE);
    }
}

void create_reindeers(pthread_t** thread_ids, struct thread_config* conf){
        for (size_t i = 1; i <= 9; i++)
    {   
        thread_ids[i] = (pthread_t*) malloc(sizeof(pthread_t));
        int tmp = pthread_create(thread_ids[i], NULL, thread_routine_reindeer, (void *)conf);

        if(tmp != 0){
            perror("Failed to start thread!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void create_elves(pthread_t** thread_ids, struct thread_config* conf){
    for (size_t i = 10; i < 20; i++)
    {
        thread_ids[i] = (pthread_t*) malloc(sizeof(pthread_t));
        int tmp = pthread_create(thread_ids[i], NULL, thread_routine_elf, (void *)conf);

        if(tmp != 0){
            perror("Failed to start thread!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void* thread_routine_santa(void* arg){
    struct thread_config* conf = (thread_config*) arg;
    int id = gettid();
    srand(time(NULL) + id);
    int delivers = 0;
    while (delivers < 3)
    {
        pthread_mutex_lock(conf->mutex);
        while (*conf->elves_in_queue != 3 && *conf->reindeers_in_queue !=9){
            pthread_cond_wait(conf->cond, conf->mutex);
        }
        printf("Mikolaj: budze sie\n");
        if(*conf->elves_in_queue == 3){
            pthread_mutex_unlock(conf->mutex);
            printf("Mikolaj: pomagam eflom: %d %d %d\n", conf->waiting_elves[0], conf->waiting_elves[1], conf->waiting_elves[2]);

            for (size_t i = 0; i < 3; i++)
            {
                sem_post(&conf->semaphores[ELVES_1]);
            }

            usleep(rand()%1000000 + 1000000);

            for (size_t i = 0; i < 3; i++)
            {
                sem_post(&conf->semaphores[ELVES_2]);
            }

            pthread_mutex_lock(conf->mutex);

            for (size_t i = 0; i < 3; i++)
            {
                conf->waiting_elves[i] = 0;
            }
            *conf->elves_in_queue = 0;
        }
        if(*conf->reindeers_in_queue == 9){
            pthread_mutex_unlock(conf->mutex);
            printf("Mikolaj: dostarczam zabawki\n");
            delivers += 1;
            usleep(rand()%2000000 + 2000000);
            for (size_t i = 0; i < 9; i++)
            {
                sem_post(&conf->semaphores[REINDEERS]);
            }
            pthread_mutex_lock(conf->mutex);

            for (size_t i = 0; i < 9; i++)
            {
                conf->waiting_reindeers[i] = 0;
            }
            *conf->reindeers_in_queue = 0;
        }
        pthread_mutex_unlock(conf->mutex);
    }

    kill(conf->parent_pid, SIGUSR1);
    
    return 0;
} 

void* thread_routine_reindeer(void* arg){
    signal(SIGINT, sig_handler_thread);
    struct thread_config* conf = (thread_config*) arg;
    int id = gettid();
    srand(time(NULL) + id);
    while(1){
        usleep(rand()%5000000 + 5000000);
        pthread_mutex_lock(conf->mutex);
        conf->waiting_reindeers[*conf->reindeers_in_queue] = id;
        *conf->reindeers_in_queue += 1;
        printf("Renifer: na mikolaja czeka %d\n", *conf->reindeers_in_queue);
        if(*conf -> reindeers_in_queue == 9){
            pthread_cond_broadcast(conf->cond);
            printf("Renifer: Budze mikolaja\n");
        }
        pthread_mutex_unlock(conf->mutex);
        sem_wait(&conf->semaphores[REINDEERS]);
    }
} 

void* thread_routine_elf(void* arg){
    signal(SIGINT, sig_handler_thread);
    struct thread_config* conf = (thread_config*) arg;
    int id = gettid();
    srand(time(NULL) + id);
    while(1){
        usleep(rand()%3000000 + 2000000);
        pthread_mutex_lock(conf->mutex);
        if(*conf->elves_in_queue < 3){
            conf->waiting_elves[*conf->elves_in_queue] = id;
            *conf->elves_in_queue += 1;
            printf("Elf %d czeka na Mikolaja\n", id);
            if(*conf -> elves_in_queue == 3){
                pthread_cond_broadcast(conf->cond);
                printf("Elf %d budzi Mikolaja\n", id);
            }
            pthread_mutex_unlock(conf->mutex);
            sem_wait(&conf->semaphores[ELVES_1]);
            printf("Elf: Mikolaj rozwiazuje problem %d\n", id);
            sem_wait(&conf->semaphores[ELVES_2]);
        }else{
            pthread_mutex_unlock(conf->mutex);
        }
    }
} 

void cleanup(pthread_t** thread_ids, sem_t* semaphores, size_t semaphores_count,
             int* waiting_elves, int* waiting_reindeers) {
    for (size_t i = 0; i < 20; i++) {
        pthread_kill(*thread_ids[i], SIGINT);
        free(thread_ids[i]);
    }
    free(thread_ids);

    for (size_t i = 0; i < semaphores_count; i++) {
        sem_destroy(&semaphores[i]);
    }

    free(semaphores);
    free(waiting_elves);
    free(waiting_reindeers);
}

void wake_up(){

}

void sig_handler_thread(){
    exit(EXIT_SUCCESS);
}