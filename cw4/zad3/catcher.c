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

void sigusr1_handler(int signum, siginfo_t * info, void * _);
void print_100_to_1();
void show_curr_time();
void show_signal_counter();
void init_sigusr1_handler();

int main(int argc, char *argv[]) {

    init_sigusr1_handler();

    printf("PID: %d\n", getpid());

    while (1)
    {
        if(is_timer_active){
            show_curr_time();
            sleep(1);
        }
    }
    
    return 0;   
}

void init_sigusr1_handler(){
    struct sigaction act;
    act.sa_sigaction = sigusr1_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
}

void timer_start(){
    is_timer_active = 1;
}

void timer_stop(){
    is_timer_active = 0;
}

void sigusr1_handler(int signum, siginfo_t * info, void * _){
    timer_stop();
    signal_counter++;

    union sigval sig;
    sig.sival_int = 0;
    
    switch (info->si_value.sival_int)
    {
        case 1:
            print_100_to_1();
            break;
        case 2:
            show_curr_time();
            break;
        case 3:
            show_signal_counter();
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

void print_100_to_1(){
    for (size_t i = 1; i <= 100; i++)
    {
        printf("%d\n", i);
    }
    fflush(stdout);
}

void show_curr_time(){
    time_t now = time(0);
    struct tm *local = localtime(&now);
    printf("Current time: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
    fflush(stdout);
}

void show_signal_counter(){
    printf("Singals received: %d\n", signal_counter);
}