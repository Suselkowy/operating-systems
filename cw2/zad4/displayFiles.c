#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>

long long total = 0;

int going_deep(const char* name, const struct stat *ptr, int flag){
    if(!S_ISDIR(ptr->st_mode)){
        printf("size: %ld, name: %s \n", ptr->st_size, name);
        total += ptr->st_size;
    }
    return 0;
}

int main( int argc, char *argv[] )  {


    if(argc < 1){
        fprintf(stderr, "Not enought arguments");
        return -1;
    }

    ftw(argv[1], going_deep, 10);
    
    printf("Total: %lld\n", total);

    return 0;
}

