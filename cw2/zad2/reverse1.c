#include "libhelpers.h"
#include <errno.h>

FILE* file_to_read;
FILE* file_to_write;

void open_files(char* input_file_path, char* output_file_path){
    file_to_read = fopen(input_file_path, "r");
    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        exit(-1);
    }

    file_to_write = fopen(output_file_path, "w+");
    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        exit(-1);
    }
}

void close_files(){
    close_file(file_to_read);
    close_file(file_to_write);
}

int reverse_file(){
    char currLetter;
    int is_valid = fseek(file_to_read, -1, SEEK_END);
    if(is_valid == -1){
        perror("Error in fseek");
        return 0;
    }
    int size = ftell(file_to_read) + 1;
    int c = 0;
    while (c < size)
    {   
        is_valid = fread(&currLetter, sizeof(char), 1, file_to_read);
        if(is_valid == 0){
            perror("Error reading file");
            return 0;  
        }
        is_valid = fwrite(&currLetter, sizeof(char), 1, file_to_write);
        if(is_valid == 0){
            perror("Error writing file");
            return 0;
        }
        is_valid = fseek(file_to_read, -2, SEEK_CUR);
        if(is_valid == -1 && size-1 != c){
            perror("Error in fseek");
            return 0;
        }
        c++;
    }
    return -1;
}

int main( int argc, char *argv[] )  {

    if(argc < 2){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    char* input_file = argv[1];
    char* output_file = argv[2];
    open_files(input_file, output_file);
    struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

    int is_valid = reverse_file();
    if(!is_valid){
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    double time = get_time(&start, &end);
    log_time(time, '1');

    close_files();
    return 0;
}

