#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

void handler(int signum){
    printf("Recived signal: %d\n", signum);
    fflush(stdout);
}

int mask_signal(){
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    if(sigprocmask(SIG_SETMASK, &new_mask, NULL) < 0){
        perror("Nie udao sie nadac maski sygnalow");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}   

int main(int argc, char *argv[]) {

    int catch_pid = atoi(argv[1]);
    int next_command;

    mask_signal();

    signal(SIGUSR1, handler);
    sigset_t new_mask;
    sigemptyset(&new_mask);

    union sigval sig;


    for(int i = 2; i < argc; i++)
    {   
        next_command = atoi(argv[i]);
        if(next_command <= 0 || next_command > 5 ){
            fprintf(stderr, "Invalid argumet provided");
            return EXIT_FAILURE;
        }
        sig.sival_int = next_command;

        sigqueue(catch_pid, SIGUSR1, sig);
        sigsuspend(&new_mask); 
        sleep(2);
    }
    

    return 0;
}

