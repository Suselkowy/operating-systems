#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

void handler(int signum);
int mask_signal();
void pending_handle();
 
int main(int argc, char *argv[]) {
    int pending = 0;
    int handle = 0;

    if(argc != 2) {
        fprintf(stdout, "Invalid number of arguments");
        return EXIT_FAILURE;
    }

    printf("Main:\n");
    if(!strncmp(argv[1], "ignore", 6)){
        signal(SIGUSR1, SIG_IGN);
    }else if(!strncmp(argv[1], "handler", 7)){
        signal(SIGUSR1, handler);
        handle = 1;
    }else if(!strncmp(argv[1], "mask", 4)){
        if(mask_signal()) return EXIT_FAILURE;
    }else if(!strncmp(argv[1], "pending", 7)){
        if(mask_signal()) return EXIT_FAILURE;
        pending = 1;
    }else{
        fprintf(stdout, "Invalid argument");
        return EXIT_FAILURE;
    }

    raise(SIGUSR1);

    if(handle != 1){
        pending_handle();
    }

    pid_t pid = fork();

    if(pid == -1){
        perror("Fork failure");
        return EXIT_FAILURE;
    }

    if(pid == 0){
        printf("Child:\n");
        if(pending != 1){
            raise(SIGUSR1);
        }
        if(handle != 1){
            pending_handle();
        }
        return EXIT_SUCCESS;
    }

    wait(NULL);

    if(pid != 0 && !handle){
        execl("./justRise", "justRise", argv[1], NULL);
        perror("Error in execl");
        return EXIT_FAILURE;
    }

    return 0;
}

void handler(int signum){
    printf("Odebrano sygnal: %d\n", signum);
    fflush(stdout);
}

int mask_signal(){
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    if(sigprocmask(SIG_SETMASK, &new_mask, NULL) < 0){
        perror("Error while adding mask");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}   

void pending_handle(){
    sigset_t set;
    sigpending(&set);
    if(sigismember(&set, SIGUSR1)){
        printf("SIGUSR1 pending\n");
    }else{
        printf("SIGUSR1 not pending\n");
    }
}