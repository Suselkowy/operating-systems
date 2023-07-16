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
double time_to_double(struct timespec* time_start, struct timespec* time_end);
double calculate_integral(double rectangle_width, int number_of_processes, int number_of_rectangles);
double aggregate_results_from_pipeline(int number_of_processes);
void log_program_stats(double time, double rectangle_width, int number_of_processes);

int main(int argc, char const *argv[])
{   
    if(argc != 3){
        fprintf(stderr, "Invalid number of agruments");
        return EXIT_FAILURE;
    }

    double rectangle_width = strtod(argv[1], NULL);
    int number_of_processes = atoi(argv[2]);

    int number_of_rectangles = floor(1.0 / rectangle_width); //TOFIX: I think that there shoud be ceil
    if(number_of_rectangles < number_of_processes){
        fprintf(stderr, "too many subprocesses - %d rectangles and %d processes\n", number_of_rectangles, number_of_processes);
        return EXIT_FAILURE;
    }

    mkfifo("./potok", S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );

    struct timespec start_t, finish_t;
    clock_gettime(CLOCK_REALTIME, &start_t);

    double final_sum = calculate_integral(rectangle_width, number_of_processes, number_of_rectangles);
    printf("Result: %lf\n", final_sum);

    clock_gettime(CLOCK_REALTIME, &finish_t);
    double time = time_to_double(&start_t, &finish_t);
    log_program_stats(time, rectangle_width, number_of_processes);

    return 0;
}

double calculate_integral(double rectangle_width, int number_of_processes, int number_of_rectangles){
    int proccesed_by_one = ceil(number_of_rectangles / number_of_processes);
    int unassigned_rectangles = number_of_rectangles - (proccesed_by_one * number_of_processes);
    double start_point = 0;
    char number_rectangles_to_process_arg[256];
    char rectangle_width_arg[256];
    char start_point_arg[256];

    for(int i = 0; i < number_of_processes; ++i){

        int number_rectangles_to_process = proccesed_by_one;
        if(unassigned_rectangles > 0){
            number_rectangles_to_process += 1;
        }

        if(fork() == 0){
            sprintf(number_rectangles_to_process_arg, "%d", number_rectangles_to_process);
            sprintf(rectangle_width_arg, "%lf", rectangle_width);
            sprintf(start_point_arg, "%lf", start_point);
            execl("./calc", "calc", number_rectangles_to_process_arg, rectangle_width_arg, start_point_arg, NULL);
            perror("Execl failed\n");
            exit(EXIT_FAILURE);
        }

        start_point += proccesed_by_one*rectangle_width;
        if(unassigned_rectangles > 0){
            start_point += rectangle_width;
            unassigned_rectangles -= 1;
        }
        
    }

    return aggregate_results_from_pipeline(number_of_processes);
}

double aggregate_results_from_pipeline(int number_of_processes){
    double final_sum  = 0;
    int pipeline_fd = open("potok", O_RDONLY);
    char line_buff[1024];
    for (size_t i = 0; i < number_of_processes; i++)
    {
        while (read(pipeline_fd, line_buff, 128) == 0);
        final_sum += strtod(line_buff, NULL);
    }
    close(pipeline_fd);
    return final_sum;
}

void log_program_stats(double time, double rectangle_width, int number_of_processes){
    char* line_buff= calloc(1024, sizeof(char));
    if(line_buff == NULL){
        perror("failed to initialize line_buff\n");
        return;
    }
    sprintf(line_buff, "time = %lf rectangle_width = %lf number_of_processes = %d\n", time, rectangle_width, number_of_processes);
    printf("%s", line_buff);

    int f = open("raport.txt", O_WRONLY|O_APPEND|O_CREAT, 0666);
    if(f < 0){
        perror("failed to open log file\n");
    }else{
        write(f, line_buff, strlen(line_buff));
        close(f);
    }
    free(line_buff);
}

double time_to_double(struct timespec* time_start, struct timespec* time_end){
	double realtime = ((double) time_end->tv_sec - time_start->tv_sec);
	double nano =  ((double) time_end->tv_nsec - time_start->tv_nsec) / (double)1000000000;
	return realtime+nano;
}