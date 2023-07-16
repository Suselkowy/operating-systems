#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

enum sort_type{DATE, SENDER};

int main(int argc, char const *argv[])
{   
    if(argc != 2 && argc != 4){
        fprintf(stderr, "Invalid number of agruments");
        return EXIT_FAILURE;
    }

    if(argc == 2){
        int sort_by;
        const char* command = argv[1];

        if(strncmp(command, "data", 4) == 0){
            sort_by = DATE;
        }else if(strncmp(command, "nadawca", 7) == 0){
            sort_by = SENDER;
        }else{
            fprintf(stderr, "Invalid agrument");
            return EXIT_FAILURE;
        }

        FILE* p;
        if(sort_by == DATE){
            p = popen("mail | tail -n +3 | head -n -1", "w");
        }else{
            p = popen("mail | tail -n +3 | head -n -1 | sort -k 3 ", "w");
        }

        fwrite("q", sizeof(char), 1, p);
        pclose(p);

    }else{
        char* line_buff = calloc(1024, sizeof(char));
        sprintf(line_buff , "mail -s %s %s", argv[2], argv[1]);
        printf("%s", line_buff);

        FILE* p = popen(line_buff, "w");
        fputs(argv[3], p); 

        pclose(p);
        free(line_buff);
    }

    return 0;
}
