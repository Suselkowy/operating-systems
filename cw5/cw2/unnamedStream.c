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
double printTimes(struct timespec* timespec_s, struct timespec* timespec_e);

int main(int argc, char const *argv[])
{   
    if(argc != 3){
        fprintf(stderr, "Invalid number of agruments");
        return EXIT_FAILURE;
    }

    char **end;
    double width = strtod(argv[1], end);
    int n = atoi(argv[2]);

    int num_of_ranges = floor(1.0 / width);
    
    if(num_of_ranges < n){
        fprintf(stderr, "too many subprocesses");
        return EXIT_FAILURE;
    }

    int proccesed_by_one = ceil(num_of_ranges / n);
    int loose = num_of_ranges - (proccesed_by_one*(n));
    
    double start = 0;
    int** fd = calloc(n, sizeof(int*));

    char* line = calloc(1024, sizeof(char));

    struct timespec start_t, finish_t;

    clock_gettime(CLOCK_REALTIME, &start_t);

    for(int i = 0; i < n ; ++i){
        fd[i] = calloc(2, sizeof(int));
        pipe(fd[i]);
        if(fork() == 0){
            int times = proccesed_by_one;
            double sum = 0;
            if(loose > 0){
                times += 1;
            }

            for (size_t j = 0; j < times; j++)
            {
                double temp_end = start+width;
                if(temp_end > 1){
                    sum += func(start+(temp_end - start)/2) * (temp_end - start);
                }else{
                    sum += func(start+(width/2)) * width;
                }
                start += width;
                
            }

            close(fd[i][0]);

            sprintf(line, "%lf", sum);
            write(fd[i][1], line, strlen(line));
            return EXIT_SUCCESS;
        }else{
            close(fd[i][1]);
            start += proccesed_by_one*width;
            if(loose > 0){
                start += width;
                loose -= 1;
            }

        }
    }
    wait(NULL);


    double final_sum  = 0;
    for(int i = 0; i < n ; ++i){
        read(fd[i][0], line, 1024);
        final_sum += strtod(line, end);
    }

    clock_gettime(CLOCK_REALTIME, &finish_t);

    double time = printTimes(&start_t, &finish_t);
    sprintf(line, "time = %lf width = %lf n = %d\n", time, width, n);
    printf("%s", line);

    int f = open("raport.txt", O_WRONLY|O_APPEND|O_CREAT, 0666);
    if(f < 0){
        perror("failed to open file");
        return EXIT_FAILURE;
    }
    write(f, line, strlen(line));
    close(f);

    return 0;
}

double func(double x){
    return 4/(x*x + 1);
}

double printTimes(struct timespec* timespec_s, struct timespec* timespec_e){
	double realtime = ((double) timespec_e->tv_sec - timespec_s->tv_sec);
	double nano =  ((double) timespec_e->tv_nsec - timespec_s->tv_nsec) / (double)1000000000;
	return realtime+nano;
}