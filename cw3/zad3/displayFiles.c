#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

const int MAX_STRING = 255;
char path[PATH_MAX];

void check_file(char* line, char* path, char* string);
int open_dir(DIR** dir_stream, char* path);
 
int main(int argc, char *argv[]) {

    if(argc != 3 ){
        fprintf(stderr, "Invalid number of arguments");
        return EXIT_FAILURE;
    }  

    if(strlen(argv[2]) > MAX_STRING){
        fprintf(stderr, "Second argument to long");
        return EXIT_FAILURE;
    }

    if(strlen(argv[1])+1 > PATH_MAX){
        fprintf(stderr, "First argument to long");
        return EXIT_FAILURE;
    }

    struct stat filestat;
    stat(argv[1], &filestat);

    if(!S_ISDIR(filestat.st_mode)){
        fprintf(stderr, "Invalid path");
        return EXIT_FAILURE;
    }else{
        char start_path[PATH_MAX];
        char line[MAX_STRING];
        pid_t pid;
        DIR* dir_stream;
        
        snprintf(path, PATH_MAX, "%s", argv[1]);
        snprintf(start_path, PATH_MAX, "%s", path);
        

        if(open_dir(&dir_stream, argv[1])){
            return EXIT_FAILURE;
        };

        struct dirent* curr_dir;

        while (curr_dir = readdir(dir_stream))
        {   

            if((strlen(path) + strlen(curr_dir->d_name) + 2) >= PATH_MAX){
                fprintf(stderr, "new path to long");
                return EXIT_FAILURE;
            }

            snprintf(path, PATH_MAX, "%s/%s", start_path, curr_dir->d_name);

            if ( stat(path, &filestat) == -1 ) {
                perror("Failed to read information with stat() ");
                return EXIT_FAILURE;
            } 

            if(S_ISDIR(filestat.st_mode)){
                if(strcmp(curr_dir->d_name, ".") != 0 && strcmp(curr_dir->d_name, "..") != 0){

                    pid = fork();

                    if(pid == -1){
                        perror("fork failure");
                        return EXIT_FAILURE;
                    }

                    if(pid == 0){
                        if(open_dir(&dir_stream, path)){
                            return EXIT_FAILURE;
                        };
                        snprintf(start_path, PATH_MAX, "%s", path);
                    }
                }
            }else{
                check_file(line, path, argv[2]);
            }
        }
    }
    return 0;
}

int open_dir(DIR** dir_stream, char* path){
    *dir_stream = opendir(path);
    if(!*dir_stream){
        perror("Error while opening dir");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void check_file(char* line, char* path, char* string){
    FILE* f = fopen(path, "r");

    if ( f == NULL) {
        fprintf(stderr, "Failure while reading the file");
        return;
    } 

    fread(line, sizeof(char), MAX_STRING, f);
    if(strncmp(line, string, strlen(string)) == 0){
        printf("path:%s, pid:%d\n", path, getpid());
    }

    fclose(f);
}