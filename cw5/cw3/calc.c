#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>


double func(double x);

int main(int argc, char const *argv[])
{   
    if(argc != 4){
        fprintf(stderr, "Invalid number of agruments\n");
        return EXIT_FAILURE;
    }

    int f = open("./potok", O_WRONLY);

    if(f < 0){
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    char **end;
    int times = atoi(argv[1]);
    double width = strtod(argv[2], end);
    double start = strtod(argv[3], end);
    double sum = 0;

    for (int j = 0; j < times; j++)
    {
        double temp_end = start+width;
        if(temp_end > 1){
            sum += func(start+(temp_end - start)/2) * (temp_end - start);
        }else{
            sum += func(start+(width/2)) * width;
        }
        start += width;
        
    }

    char line[128];
    sprintf(line, "%lf", sum);
    write(f, line, 128);
    close(f);

    return 0;
}

double func(double x){
    return 4/(x*x + 1);
}
