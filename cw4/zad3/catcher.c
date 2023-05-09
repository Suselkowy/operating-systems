#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>
#include <time.h>

int signal_counter = 0;
int is_timer_active = 0;

void handler(int signum, siginfo_t * info, void * _);
void count();
void timed();
void print_signal_counter();
 
int main(int argc, char *argv[]) {
    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    printf("%d", getpid());
    fflush(stdout);

    while (1)
    {
        if(is_timer_active){
            timed();
            sleep(1);
        }
    }
    

    return 0;   
}

void timer_start(){
    is_timer_active = 1;
}

void timer_stop(){
    is_timer_active = 0;
}

void handler(int signum, siginfo_t * info, void * _){
    timer_stop();
    union sigval sig;
    sig.sival_int = 0;
    signal_counter += 1;

    switch (info->si_value.sival_int)
    {
    case 1:
        count();
        break;
    case 2:
        timed();
        break;
    case 3:
        print_signal_counter();
        break;
    case 4:
        timer_start();
        break;
    case 5:
        sigqueue(info->si_pid, SIGUSR1, sig);
        exit(EXIT_SUCCESS);
    default:
        break;
    }

    fflush(stdout);

    sigqueue(info->si_pid, SIGUSR1, sig);
}

void count(){
    for (size_t i = 1; i <= 100; i++)
    {
        printf("%d\n", i);
    }
    fflush(stdout);
}

void timed(){
    time_t now = time(0);
    struct tm *local = localtime(&now);
    printf("Current time: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
    fflush(stdout);
}

void print_signal_counter(){
    printf("%d\n", signal_counter);
}