#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

const int MAX_STRING_LENGTH = 255;

void check_file(char* file_line_buff, char* path, char* string);
int open_dir(DIR** dir_stream, char* path);
void fork_and_traverse_directory(DIR** dir_stream, char* curr_path, char* start_path);
 
int main(int argc, char *argv[]) {
    if(argc != 3 ){
        fprintf(stderr, "Invalid number of arguments - expected 2, provided %d", argc-1);
        return EXIT_FAILURE;
    }  

    const char* string_to_find = argv[2];
    if(strlen(string_to_find) > MAX_STRING_LENGTH){
        fprintf(stderr, "Second argument to long - max length: %d", MAX_STRING_LENGTH);
        return EXIT_FAILURE;
    }

    const char* starting_path = argv[1];
    if(strlen(starting_path) > PATH_MAX){
        fprintf(stderr, "First argument to long");
        return EXIT_FAILURE;
    }

    char start_path[PATH_MAX + 1];
    char curr_path[PATH_MAX + 1];
    char file_line_buff[MAX_STRING_LENGTH + 1];
    snprintf(start_path, PATH_MAX, "%s", starting_path);
    snprintf(curr_path, PATH_MAX, "%s", starting_path);

    struct stat filestat;
    if (stat(starting_path, &filestat) == -1) {
        perror("Failed to read information with stat()");
        return EXIT_FAILURE;
    }

    if(!S_ISDIR(filestat.st_mode)){
        fprintf(stderr, "Invalid path");
        return EXIT_FAILURE;
    }else{
        DIR* dir_stream;  
        if(open_dir(&dir_stream, start_path) == -1){
            return EXIT_FAILURE;
        };

        pid_t pid;
        struct dirent* curr_dir;
        while (curr_dir = readdir(dir_stream))
        {   
            if((strlen(curr_path) + strlen("/") + strlen(curr_dir->d_name)  + 1) > PATH_MAX){
                fprintf(stderr, "new path too long");
                return EXIT_FAILURE;
            }
            snprintf(curr_path, PATH_MAX, "%s/%s", start_path, curr_dir->d_name);

            if ( stat(curr_path, &filestat) == -1 ) {
                fprintf(stderr, "Failed to read information about %s", curr_path);
                return EXIT_FAILURE;
            } 

            if(S_ISDIR(filestat.st_mode)){
                if(strcmp(curr_dir->d_name, ".") != 0 && strcmp(curr_dir->d_name, "..") != 0){
                    fork_and_traverse_directory(&dir_stream, curr_path, start_path);
                }
            }else{
                check_file(file_line_buff, curr_path, (char*)string_to_find);
            }
        }
    }
    return 0;
}

int open_dir(DIR** dir_stream, char* path){
    *dir_stream = opendir(path);
    if(!*dir_stream){
        perror("Error while opening dir");
        return -1;
    }
    return 0;
}

void check_file(char* file_line_buff, char* path, char* string){
    FILE* f = fopen(path, "r");
    if ( f == NULL) {
        fprintf(stderr, "Failure opening file %s", path);
        return;
    } 
    fread(file_line_buff, sizeof(char), MAX_STRING_LENGTH, f);
    if(strncmp(file_line_buff, string, strlen(string)) == 0){
        printf("path:%s, pid:%d\n", path, getpid());
    }

    fclose(f);
}

void fork_and_traverse_directory(DIR** dir_stream, char* curr_path, char* start_path) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failure");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (open_dir(dir_stream, curr_path) == -1) {
            exit(EXIT_FAILURE);
        }
        snprintf(start_path, PATH_MAX, "%s", curr_path);
    } else {
        wait(NULL);
    }
}