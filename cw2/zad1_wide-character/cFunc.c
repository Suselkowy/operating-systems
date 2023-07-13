#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

FILE* file_to_read;
FILE* file_to_write;

void close_file(FILE* f){
    fclose(f);
}

void handle_mbtowc_error(){
    fprintf(stderr, "Invalid multibyte sequence");
    close_files();
    exit(-1);
}

void open_files(const char* input_file, const char* output_file) {
    file_to_read = fopen(input_file, "r");
    if (file_to_read == NULL){
        fprintf(stderr, "Failed to open first file");
        exit(-1);
    }

    file_to_write = fopen(output_file, "w");
    if (file_to_write == NULL){
        fprintf(stderr, "Failed to open second file");
        close_file(file_to_read);
        exit(-1);
    }
}

void close_files() {
    close_file(file_to_read);
    close_file(file_to_write);
}

void perform_replacement(const wchar_t* letter_to_find, const wchar_t* letter_to_insert) {
    wint_t currLetter;
    while ((currLetter = fgetwc(file_to_read)) != WEOF) {
        if(currLetter == *letter_to_find){
            currLetter = *letter_to_insert;
        }
        fputwc(currLetter, file_to_write);
    }
}

size_t multibyte_char_to_wint_t(wint_t* wint, char* multibyte_char){
    size_t returnValue = mbtowc(&wint, multibyte_char, sizeof(wchar_t));
    return returnValue;
}

int main( int argc, char *argv[] )  {

    setlocale(LC_ALL, "");

    if(argc < 4){
        fprintf(stderr, "Not enough parameters provided");
        return -1;
    }
    
    const char* input_file = argv[3];
    const char* output_file = argv[4];
    open_files(input_file, output_file);

    wint_t letter_to_find;
    if (multibyte_char_to_wint_t(&letter_to_find, argv[1]) == (size_t)-1) {
        handle_mbtowc_error();
    }

    wint_t letter_to_insert;
    if (multibyte_char_to_wint_t(&letter_to_insert, argv[2]) == (size_t)-1) {
        handle_mbtowc_error();
    }

    perform_replacement(&letter_to_find, &letter_to_insert);
     
    close_files();


    return 0;
}
