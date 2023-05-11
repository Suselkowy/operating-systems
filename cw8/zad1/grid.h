#pragma once
#include <stdbool.h>
#include <pthread.h>

typedef struct thread_config{
    int cell_row;
    int cell_col;
    char* source;
    char* dest;  
}thread_config;

char *create_grid();
void destroy_grid(char *grid);
void draw_grid(char *grid);
void init_grid(char *grid);
bool is_alive(int row, int col, char *grid);
void update_grid(char *src, char *dst, pthread_t** thread_ids);
pthread_t **create_threads(char *src, char *dst, thread_config** config_tab);
void destroy_pointer_tab(void** tab);
thread_config** create_config_tab();


