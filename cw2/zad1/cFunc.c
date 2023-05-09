#include <stdio.h>
#include <stdlib.h>

void close_file(FILE* f){
    fclose(f);
}

int main( int argc, char *argv[] )  {

    if(argc < 4){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    FILE* file_to_read = fopen(argv[3], "r");

    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        return -1;
    }

    FILE* file_to_write = fopen(argv[4], "w");


    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        return -1;
    }

    char currLetter;
    size_t last_read = 0;
    last_read = fread(&currLetter, sizeof(char), 1, file_to_read);
    
    while (last_read > 0)
    {
        if(currLetter == *argv[1]){
            currLetter = *argv[2];
        }
        fwrite(&currLetter, sizeof(char), 1, file_to_write);

        last_read = fread(&currLetter, 1, 1, file_to_read);
    }
     
    close_file(file_to_read);
    close_file(file_to_write);


    return 0;
}
