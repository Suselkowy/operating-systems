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
        pause();
        conf->dest[conf->cell_row * grid_width + conf->cell_col] = is_alive(conf->cell_row, conf->cell_col, conf->source);

        char* tmp = conf->source;
		conf->source = conf->dest;
		conf->dest = tmp;
    }
    return 0;
} 

thread_config** create_config_tab(){
    return malloc(sizeof(thread_config*) * grid_width * grid_width);
}

pthread_t **create_threads(char *src, char *dst, thread_config** config_tab){
    pthread_t** thread_ids = malloc(sizeof(pthread_t*) * grid_height * grid_width);
    for (size_t i = 0; i < grid_height; i++)
    {
        for (size_t j = 0; j < grid_width; j++)
        {
            struct thread_config* conf = calloc(sizeof(thread_config), 1);
            config_tab[i] = conf;

            conf->cell_row = i;
            conf->cell_col = j;
            conf->dest = dst;
            conf->source = src;
            thread_ids[i*grid_width + j] = (pthread_t*) malloc(sizeof(pthread_t));
            int tmp = pthread_create(thread_ids[i*grid_width + j], NULL, thread_routine, (void *) conf);

            if(tmp != 0){
                perror("Failed to start thread!");
                exit(EXIT_FAILURE);
            }
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

void update_grid(char *src, char *dst, pthread_t** thread_ids)
{
    for (int i = 0; i < grid_height; ++i)
    {
        for (int j = 0; j < grid_width; ++j)
        {   
            pthread_kill(*thread_ids[i * grid_width + j] , SIGUSR1);
            //dst[j * grid_width + i] = is_alive(j, i, src);
        }
    }
}

void destroy_pointer_tab( void** tab){
    int n = grid_width * grid_height;
    for (size_t i = 0; i < n; i++)
    {
        free(tab[i]);
    }
    free(tab);
}