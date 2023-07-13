#include <stdio.h>
#include <stdlib.h>

FILE* file_to_read;
FILE* file_to_write;

void close_file(FILE* f){
    fclose(f);
}

void close_files() {
    close_file(file_to_read);
    close_file(file_to_write);
}

void open_files(char* input_file, char* output_file){
    file_to_read = fopen(input_file, "r");
    if(file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        exit(-1);
    }

    file_to_write = fopen(output_file, "w");
    if(file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        exit(-1);
    }
}

void perform_replacement(const char* letter_to_find, const char* letter_to_insert) {
    char currLetter; 
    while (fread(&currLetter, sizeof(char), 1, file_to_read) > 0)
    {
        if(currLetter == *letter_to_find){
            currLetter = *letter_to_insert;
        }
        fwrite(&currLetter, sizeof(char), 1, file_to_write);
    }
}

int main( int argc, char *argv[] )  {

    if(argc < 4){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    const char* input_file = argv[3];
    const char* output_file = argv[4];
    open_files(input_file, output_file);

    const char* letter_to_find = argv[1];
    const char* letter_to_insert = argv[2];

    perform_replacement(letter_to_find, letter_to_insert);
    close_files();
    return 0;
}
