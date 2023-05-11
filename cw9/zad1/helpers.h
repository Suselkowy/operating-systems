enum semaphore_name{
    ELVES_1 = 0,
    ELVES_2 = 1,
    REINDEERS = 2,
};

typedef struct thread_config{
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* waiting_reindeers;
    int* reindeers_in_queue;
    int* waiting_elves;
    int* elves_in_queue;
    sem_t* semaphores; 
    pid_t parent_pid;
}thread_config;
