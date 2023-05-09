#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "libhelpers.h"

int main( int argc, char *argv[] )  {

    if(argc < 2){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    FILE* file_to_read = fopen(argv[1], "r");

    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        return -1;
    }

    FILE* file_to_write = fopen(argv[2], "w+");


    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        return -1;
    }

    struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

    char currLetter;
    fseek(file_to_read, -1, SEEK_END);
    int size = ftell(file_to_read) + 1;
    int c = 0;
    while (c < size)
    {   
        fread(&currLetter, sizeof(char), 1, file_to_read);
        fwrite(&currLetter, sizeof(char), 1, file_to_write);
        fseek(file_to_read, -2, SEEK_CUR);
        c++;
    }

    clock_gettime(CLOCK_REALTIME, &end);

    double time = get_time(&start, &end);


    save_to_file(time, '1');

    close_file(file_to_read);
    close_file(file_to_write);


    return 0;
}

