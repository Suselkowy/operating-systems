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
#include <errno.h>
#define PIPELINE_FILE_PATH "./potok"

double integral_function(double x);

int main(int argc, char const *argv[])
{   
    if(argc != 4){
        fprintf(stderr, "Invalid number of agruments: 3 expected, %d provided\n", argc-1);
        return EXIT_FAILURE;
    }

    int pipeline_fd = open(PIPELINE_FILE_PATH, O_WRONLY);
    if(pipeline_fd < 0){
        fprintf(stderr, "%s: Failed to open file %s\n", strerror(errno), PIPELINE_FILE_PATH);
        return EXIT_FAILURE;
    }

    int number_rectangles_to_process = atoi(argv[1]);
    double rectangle_width = strtod(argv[2], NULL);
    double start_point = strtod(argv[3], NULL);
    double sum = 0;

    for (int j = 0; j < number_rectangles_to_process; j++)
    {
        double temp_end = start_point+rectangle_width;
        if(temp_end > 1){
            sum += integral_function(start_point+(temp_end - start_point)/2) * (temp_end - start_point);
        }else{
            sum += integral_function(start_point+(rectangle_width/2)) * rectangle_width;
        }
        start_point += rectangle_width;
        
    }

    char line[128];
    sprintf(line, "%lf", sum);
    write(pipeline_fd, line, 128);

    close(pipeline_fd);
    return 0;
}

double integral_function(double x){
    return 4/(x*x + 1);
}
