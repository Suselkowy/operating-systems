#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int file_to_read, file_to_write;

void close_file(int f){
    close(f);
}

void close_files(){
    close_file(file_to_read);
    close_file(file_to_write);
}

void perform_replacement(char* letter_to_find, char* letter_to_insert){
    char currLetter;
    while (read(file_to_read, &currLetter, 1) == 1)
    {   
        if(currLetter == *letter_to_find){
            currLetter = *letter_to_insert;
        }
        write(file_to_write, &currLetter, 1);
    }
}

int main( int argc, char *argv[] )  {

    if(argc < 4){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }

    file_to_read = open(argv[3], O_RDONLY);
    if(file_to_read == -1){
        fprintf(stderr, "Failed to open first file");
        exit(-1);
    }

    file_to_write = open(argv[4], O_WRONLY|O_CREAT);
    if (file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        exit(-1);
    }

    char* letter_to_find = argv[1];
    char* letter_to_insert = argv[2];
    perform_replacement(letter_to_find, letter_to_insert);
     
    close_files();

    return 0;
}