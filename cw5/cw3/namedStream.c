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

    mkfifo("./potok", S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );

    int proccesed_by_one = ceil(num_of_ranges / n);
    int loose = num_of_ranges - (proccesed_by_one*(n));
    
    double start = 0;
    int** fd = calloc(n, sizeof(int*));

    char* line = calloc(1024, sizeof(char));

    struct timespec start_t, finish_t;

    clock_gettime(CLOCK_REALTIME, &start_t);

    char times_arg[256];
    char width_arg[256];
    char start_arg[256];

    for(int i = 0; i < n ; ++i){

        int times = proccesed_by_one;
        if(loose > 0){
            times += 1;
        }

        if(fork() == 0){
            sprintf(times_arg, "%d", times);
            sprintf(width_arg, "%lf", width);
            sprintf(start_arg, "%lf", start);
            execl("./calc", "calc", times_arg, width_arg, start_arg, NULL);
            perror("Execl failed");
            return EXIT_FAILURE;
        }

        start += proccesed_by_one*width;
        if(loose > 0){
            start += width;
            loose -= 1;
        }
        
    }

    double final_sum  = 0;

    int fa = open("potok", O_RDONLY);


    for (size_t i = 0; i < n; i++)
    {
        while (read(fa, line, 128) == 0);
        final_sum += strtod(line, end);
    }
    
    close(fa);
    clock_gettime(CLOCK_REALTIME, &finish_t);
    double time = printTimes(&start_t, &finish_t);
    printf("final sum = %lf\n", final_sum);


    char* to_write = calloc(1024, sizeof(char));
    sprintf(to_write, "time = %lf width = %lf n = %d\n", time, width, n);

    int f = open("raport.txt", O_WRONLY|O_APPEND|O_CREAT, 0666);
    if(f < 0){
        perror("failed to open file");
        return EXIT_FAILURE;
    }
    write(f, to_write, strlen(to_write));
    close(f);

    return 0;
}


double printTimes(struct timespec* timespec_s, struct timespec* timespec_e){
	double realtime = ((double) timespec_e->tv_sec - timespec_s->tv_sec);
	double nano =  ((double) timespec_e->tv_nsec - timespec_s->tv_nsec) / (double)1000000000;
	return realtime+nano;
}