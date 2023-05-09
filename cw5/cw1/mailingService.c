#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{   
    if(argc != 2 && argc != 4){
        fprintf(stderr, "Invalid number of agruments");
        return EXIT_FAILURE;
    }

    
    if(argc == 2){
        int read_type;

        if(strncmp(argv[1], "data", 4) == 0){
            read_type = 0;
        }else if(strncmp(argv[1], "nadawca", 7) == 0){
            read_type = 1;
        }else{
        fprintf(stderr, "Invalid agrument");
        return EXIT_FAILURE;
        }
        char* line = calloc(1024, sizeof(char));
        FILE* p;

        if(!read_type){
            p = popen("mail | tail -n +3 | head -n -1", "w");
        }else{
            p = popen("mail | tail -n +3 | head -n -1 | sort -k 3 ", "w");
        }

        fwrite("q", sizeof(char), 1, p);
        pclose(p);
        free(line);
    }else{
        char* line = calloc(1024, sizeof(char));
        sprintf(line, "mail -s %s %s", argv[2], argv[1]);
        printf("%s", line);
        FILE* p = popen(line, "w");
        fputs(argv[3], p);
        pclose(p);
    }



    return 0;
}
