#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
 
int main(int argc, char *argv[]) {

    if(argc != 2 ){
        fprintf(stderr, "Invalid number of arguments - expected 1, provided %d", argc);
        return 1;
    }    

    printf("%s ", argv[0]);
    fflush(stdout);

    execl("/bin/ls", "ls", argv[1], NULL);

    perror("exelc failure");
    return EXIT_FAILURE;
}

