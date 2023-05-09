#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

void pending_handle(){
    sigset_t set;
    sigpending(&set);
    if(sigismember(&set, SIGUSR1)){
        printf("SIGUSR1 pending\n");
    }else{
        printf("SIGUSR1 not pending\n");
    }
}
 
int main(int argc, char *argv[]) {
    int pending = 0;

    printf("Execl:\n");
    if(argc != 2) return EXIT_FAILURE;
    if(!strncmp(argv[1], "pending", 7)){
        pending = 1;
    }
    if(pending != 1){
        raise(SIGUSR1);
    }

    pending_handle();

    return 0;

}

