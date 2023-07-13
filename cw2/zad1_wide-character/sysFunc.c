#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void close_file(int f){
    close(f);
}

int main( int argc, char *argv[] )  {

    if(argc < 4){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    int file_to_read, file_to_write;

    file_to_read = open(argv[3], O_RDONLY);

    file_to_write = open(argv[4], O_WRONLY|O_CREAT);

    char currLetter;
    
    while (read(file_to_read, &currLetter, 1) == 1)
    {   
        if(currLetter == *argv[1]){
            currLetter = *argv[2];
        }
        write(file_to_write, &currLetter, 1);
    }
     
    close_file(file_to_read);
    close_file(file_to_write);

    return 0;
}