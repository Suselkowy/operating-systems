#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
 
int main(int argc, char *argv[]) {
    if(argc != 2 ){
        fprintf(stderr, "Invalid number of arguments");
        return EXIT_FAILURE;
    }    

    int num = atoi(argv[1]);
    if(num <= 0 ){
        fprintf(stderr, "Invalid number provided");
        return EXIT_FAILURE;
    }

    pid_t pid;
    for (size_t i = 0; i < num; i++)
    {
        pid = fork();
        if(pid == -1){
            perror("Fork failed");
            return EXIT_FAILURE;
        }
        
        if(pid == 0){
            printf("PPID: %d PID: %d\n", getppid(), getpid());
            return EXIT_SUCCESS;
        }

        wait(NULL);
    }

    printf("%s", argv[1]);
    return EXIT_SUCCESS;
}

