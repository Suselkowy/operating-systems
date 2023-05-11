#include "grid.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

const int grid_width = 30;
const int grid_height = 30;


char *create_grid()
{
    return malloc(sizeof(char) * grid_width * grid_height);
}

void skip(){}

void* thread_routine(void* arg){
    signal(SIGUSR1, skip);
    thread_config* conf = (thread_config*) arg;

    while(1){
        for (size_t i = conf->cell_start; i < conf->cell_end; i++)
        {   
            conf->dest[i] = is_alive(i / grid_width, i % grid_height, conf->source);
        }
        
        char* tmp = conf->source;
		conf->source = conf->dest;
		conf->dest = tmp;
        pause();
    }
    return 0;
} 

pthread_t **create_threads(char *src, char *dst, int n, thread_config** config_tab){
    pthread_t** thread_ids = malloc(sizeof(pthread_t*) * n);

    int cells_in_grid = grid_width * grid_height;

    int cells_per_thread = cells_in_grid / n;
    int rest = cells_in_grid - cells_per_thread*n;
    int start = 0;

    for (size_t i = 0; i < n; i++)
    {
        struct thread_config* conf = malloc(sizeof(thread_config));
        config_tab[i] = conf;

        conf->cell_start = start;
        if(rest > 0){
            start += cells_per_thread + 1;
            --rest;
        }else{
            start += cells_per_thread;
        }
        conf->cell_end = start;
        //printf("s: %d, e: %d\n", conf->cell_start, conf->cell_end);

        conf->dest = dst;
        conf->source = src;
        thread_ids[i] = (pthread_t*) malloc(sizeof(pthread_t));
        int tmp = pthread_create(thread_ids[i], NULL, thread_routine, (void *) conf);

        if(tmp != 0){
            perror("Failed to start thread!");
            exit(EXIT_FAILURE);
        }
    }
    return thread_ids;
} 

void destroy_grid(char *grid)
{
    free(grid);
}

void draw_grid(char *grid)
{
    for (int i = 0; i < grid_height; ++i)
    {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j)
        {
            if (grid[i * grid_width + j])
            {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            }
            else
            {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void init_grid(char *grid)
{
    for (int i = 0; i < grid_width * grid_height; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid)
{

    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width)
            {
                continue;
            }
            if (grid[grid_width * r + c])
            {
                count++;
            }
        }
    }

    if (grid[row * grid_width + col])
    {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    }
    else
    {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst, pthread_t** thread_ids, int n)
{
    for (int i = 0; i < n; ++i)
    {

        pthread_kill(*thread_ids[i] , SIGUSR1);

    }
}

void destroy_pointer_tab( void** tab, int n){
    for (size_t i = 0; i < n; i++)
    {
        free(tab[i]);
    }
    free(tab);
}