#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/stat.h>

int main( int argc, char *argv[] )  {

    DIR* dir_stream = opendir(".");
    if(dir_stream == NULL){
        fprintf(stderr, "Failed to open directory");
        return -1;
    }

    struct dirent* curr_dir;
    struct stat filestat;
    long long total = 0;
    while (curr_dir = readdir(dir_stream))
    {   
        stat(curr_dir->d_name, &filestat);
        if( !S_ISDIR(filestat.st_mode)){
            printf("size: %d, name: %s\n",filestat.st_size, curr_dir->d_name);
            total += filestat.st_size;
        }
    }
    printf("Total: %lld\n", total);
    

    return 0;
}

