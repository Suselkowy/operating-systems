#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

int inside;
int num = 0;

void handler(int signum, siginfo_t * info, void * _){
    printf("Recived signal: %d\n", signum);
    printf("from process: %d\n", info->si_pid); 
    printf("signal value: %d\n", info->si_value);
    printf("signal code: %d\n", info->si_code);
    printf("addr: %lu\n", info->si_addr);
    printf("\n");
    fflush(stdout);
}

void handler2(int singum, siginfo_t * info, void * _){
    printf("Handle SIGTERM\n");
    fflush(stdout);
}

void handler3(int signum, siginfo_t * info, void * _){
    if(inside == 0){
        inside = 1;
    }else{
        printf("We are inside :O\n");
        return;
    }

    printf("Recived signal: %d\n", signum);
    if(num == 0){
        ++num;
        raise(SIGUSR2);
    }
    printf("Handled\n");
    fflush(stdout);
    inside = 0;
}

int main(int argc, char *argv[]) {
    printf("SA_SIGINFO\n");
    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    raise(SIGUSR1);

    pid_t pid = fork();

    if(pid == 0){
        raise(SIGUSR2);
        return(EXIT_SUCCESS);
    }else{
        union sigval sig;
        sig.sival_int = 123;
        sigqueue(pid, SIGUSR2, sig);
    }

    wait(NULL);

    printf("SA_NODEFER\n");
    printf("without flag\n");
    struct sigaction act2;
    act2.sa_sigaction = handler3;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = 0;

    sigaction(SIGUSR2, &act2, NULL);
    raise(SIGUSR2);
    num = 0;

    printf("\nwith flag\n");
    act2.sa_sigaction = handler3;
    act2.sa_flags = SA_NODEFER;

    sigaction(SIGUSR2, &act2, NULL);
    raise(SIGUSR2);

    printf("\nSA_RESETHAND\n");
    act.sa_flags = SA_RESETHAND;
    act.sa_sigaction = handler2;
    sigaction(SIGTERM, &act, NULL);

    printf("rise SIGTERM\n");
    raise(SIGTERM);
    printf("rise SIGTERM\n");
    raise(SIGTERM);    
    return 0;
}

