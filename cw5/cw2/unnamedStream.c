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

double integral_function(double x);
double time_to_double(struct timespec* timespec_s, struct timespec* timespec_e);
void log_program_stats(double time, double rectangle_width, int number_of_processes);
double calculate_integral(double rectangle_width, int number_of_processes, int number_of_rectangles);
void free_pipelines(int** pipelines, int number_of_processes);
double aggregate_results_from_pipelines(int** pipelines, int number_of_processes);
void calculate_partial_intergal(int* pipeline, int process_number, int number_of_processes, int proccesed_by_one, int unassigned_rectangles, double rectangle_width);

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
    int unassigned_rectangles = number_of_rectangles - (proccesed_by_one*number_of_processes);
    double start = 0;
    int** pipelines = calloc(number_of_processes, sizeof(int*));
    if(pipelines == NULL){
        perror("Allocating pipelines failed\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < number_of_processes ; ++i){
        pipelines[i] = calloc(2, sizeof(int));
        if(pipelines[i] == NULL){
            free_pipelines(pipelines, number_of_processes);
            exit(EXIT_FAILURE);
        }
        pipe(pipelines[i]);

        if(fork() == 0){
            calculate_partial_intergal(pipelines[i], i, number_of_processes, proccesed_by_one, unassigned_rectangles, rectangle_width);
        }else{
            close(pipelines[i][1]);

            start += proccesed_by_one*rectangle_width;
            if(unassigned_rectangles > 0)
            {
                start += rectangle_width;
                unassigned_rectangles -= 1;
            }
        }
    }
    while(wait(NULL) > 0);

    double final_sum = aggregate_results_from_pipelines(pipelines, number_of_processes);

    free_pipelines(pipelines, number_of_processes);
    return final_sum;
}

void calculate_partial_intergal(int* pipeline, int process_number, int number_of_processes, int proccesed_by_one, int unassigned_rectangles, double rectangle_width){
    close(pipeline[0]);

    int to_process = proccesed_by_one;
    double sum = 0;

    double start = process_number * proccesed_by_one * rectangle_width;
    if(unassigned_rectangles > 0){
        to_process += 1;
    }

    for (size_t j = 0; j < to_process; j++)
    {
        double temp_end = start+rectangle_width;
        if(temp_end > 1){
            sum += integral_function(start+(temp_end - start)/2) * (temp_end - start);
        }else{
            sum += integral_function(start+(rectangle_width/2)) * rectangle_width;
        }
        start += rectangle_width;
    }

    char line_buff[1024];
    sprintf(line_buff, "%lf", sum);
    write(pipeline[1], line_buff, strlen(line_buff));
    exit(EXIT_SUCCESS);
}

double aggregate_results_from_pipelines(int** pipelines, int number_of_processes){
    double final_sum  = 0;
    char line_buff[1024];
    for(int i = 0; i < number_of_processes ; ++i){
        read(pipelines[i][0], line_buff, 1024);
        final_sum += strtod(line_buff, NULL);
    }
}

void free_pipelines(int** pipelines, int number_of_processes){
    if(pipelines != NULL){
        for (size_t i = 0; i < number_of_processes; i++)
        {
            if(pipelines[i] != NULL) free(pipelines[i]);
        }
        free(pipelines);
    }
}

void log_program_stats(double time, double rectangle_width, int number_of_processes){
    char* line_buff = calloc(1024, sizeof(char));
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

double integral_function(double x){
    return 4/(x*x + 1);
}

double time_to_double(struct timespec* time_start, struct timespec* time_end){
	double realtime = ((double) time_end->tv_sec - time_start->tv_sec);
	double nano =  ((double) time_end->tv_nsec - time_start->tv_nsec) / (double)1000000000;
	return realtime+nano;
}