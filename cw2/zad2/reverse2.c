#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "libhelpers.h"

FILE* file_to_read;
FILE* file_to_write;

void close_files() {
    if (file_to_read != NULL) {
        fclose(file_to_read);
        file_to_read = NULL;
    }
    if (file_to_write != NULL) {
        fclose(file_to_write);
        file_to_write = NULL;
    }
}

void open_files(char* input_file_path, char* output_file_path){
    file_to_read = fopen(input_file_path, "r");
    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        exit(-1);
    }

    file_to_write = fopen(output_file_path, "w+");
    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_files();
        exit(-1);
    }
}

void reverse_file(){
    char* char_buff = calloc(1024, sizeof(char));
    if (char_buff == NULL) {
        perror("Memory allocation failed");
        close_files();
        exit(EXIT_FAILURE);
    }

    fseek(file_to_read, 0, SEEK_END);
    int pos = ftell(file_to_read);
    int num_chars_to_read;

    while ( pos > 0 )
    {   
        num_chars_to_read = (pos > 1024) ? 1024 : pos;
        fseek(file_to_read, -num_chars_to_read, SEEK_CUR);
        fread(char_buff, sizeof(char), num_chars_to_read, file_to_read);

        char* tmp = char_buff + num_chars_to_read - 1;
        for (size_t i = 0; i < num_chars_to_read; i++)
        {
            fwrite(tmp, sizeof(char), 1, file_to_write);
            --tmp;
        }
        fseek(file_to_read, -num_chars_to_read, SEEK_CUR);
        pos -= num_chars_to_read;
    }

    free(char_buff);
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

    reverse_file();

    clock_gettime(CLOCK_REALTIME, &end);
    double time = get_time(&start, &end);
    log_time(time, '2');

    close_files();
    return 0;
}

