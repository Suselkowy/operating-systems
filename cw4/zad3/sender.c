#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

void sigusr1_handler(int signum);
void mask_sigusr1_signal();

int main(int argc, char *argv[]) {
    if(argc < 3){
        fprintf(stderr, "Not enough arguments provided. Expected at least 2, %d provided.\n Function call: ./sender PID Command [Command...]\n", argc - 1);
        return EXIT_FAILURE;
    }

    int catch_pid = atoi(argv[1]);
    if(kill(catch_pid, 0) == -1){
        fprintf(stderr, "Process with PID %d deos not exists\n", catch_pid);
        return EXIT_FAILURE;
    }

    mask_sigusr1_signal();
    signal(SIGUSR1, sigusr1_handler);

    sigset_t empty_mask;
    sigemptyset(&empty_mask);

    union sigval sig;
    int next_command;
    for(int i = 2; i < argc; i++)
    {   
        next_command = atoi(argv[i]);
        if(next_command <= 0 || next_command > 5 ){
            fprintf(stderr, "Invalid argumet provided\n");
            continue;
        }
        sig.sival_int = next_command;

        sigqueue(catch_pid, SIGUSR1, sig);
        sigsuspend(&empty_mask); 
        sleep(2);
    }
    

    return 0;
}

void sigusr1_handler(int signum){
    printf("Recived SIGUSR1\n");
    fflush(stdout);
}

void mask_sigusr1_signal(){
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    if(sigprocmask(SIG_SETMASK, &new_mask, NULL) < 0){
        perror("Nie udao sie nadac maski sygnalow\n");
        exit(EXIT_FAILURE);
    }
}   