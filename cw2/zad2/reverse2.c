#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "libhelpers.h"


int shift(int pos){
    if(pos > 2048){
        return -1024;
    }
    return -(pos - 1024);
}

int main( int argc, char *argv[] )  {

    if(argc < 2){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    FILE* create = fopen(argv[2],"w+");
    fclose(create);

    FILE* file_to_read = fopen(argv[1], "r");

    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        return -1;
    }

    FILE* file_to_write = fopen(argv[2], "a");


    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        return -1;
    }

    struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

    char *currLetter = calloc(1024, sizeof(char));

    fseek(file_to_read, 0, SEEK_END);

    int file_size = ftell(file_to_read);
    int iterations = (file_size / 1024);

    if(file_size >= 1024){
        fseek(file_to_read, -1024, SEEK_END);
    }else{
        fseek(file_to_read, 0, SEEK_SET);
    }

    int tmp = fread(currLetter, sizeof(char), 1024, file_to_read);

    int c = 0;
    int nshif = -tmp;

    while ( c <= iterations)
    {   

        char* tmp = currLetter + -nshif - 1;
        for (size_t i = 0; i < -nshif; i++)
        {
            fwrite(tmp, sizeof(char), 1, file_to_write);
            --tmp;
        }

        nshif = shift(ftell(file_to_read));

        fseek(file_to_read, -1024 + nshif, SEEK_CUR);
        fread(currLetter, sizeof(char), 1024, file_to_read);
        ++c;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    double time = get_time(&start, &end);

    log_time(time, '2');
    close_file(file_to_read);
    close_file(file_to_write);


    return 0;
}

