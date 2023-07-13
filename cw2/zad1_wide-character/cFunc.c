#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

void close_file(FILE* f){
    fclose(f);
}

int main( int argc, char *argv[] )  {

    setlocale(LC_ALL, "");

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
    wint_t letter_to_find;
    mbtowc(&letter_to_find, argv[1], sizeof(wchar_t));

    wint_t letter_to_insert = *argv[2];
    mbtowc(&letter_to_insert, argv[2], sizeof(wchar_t));

    wint_t currLetter;
    while ((currLetter = fgetwc(file_to_read)) != WEOF)
    {   
        
        if(currLetter == letter_to_find){
            currLetter = letter_to_insert;
        }
        fputwc(currLetter, file_to_write);
    }
     
    close_file(file_to_read);
    close_file(file_to_write);


    return 0;
}
