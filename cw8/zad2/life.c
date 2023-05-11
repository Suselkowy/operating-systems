#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <pthread.h>

int running = 1;

void stop_running(){
	running = 0;
	return;
}

int main(int argc, char* argv[])
{

	signal(SIGINT, stop_running);

	if (argc != 2){
		fprintf(stderr, "Not enough paramteres!");
		exit(EXIT_FAILURE);
	}

	int n = atoi(argv[1]);

	if (n <= 0){
		fprintf(stderr, "Invalid parameters!");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	thread_config** config_tab = malloc(sizeof(thread_config*) * n);
	pthread_t** thread_ids = create_threads(foreground, background, n, config_tab);

	init_grid(foreground);

	while (running)
	{
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		update_grid(foreground, background, thread_ids, n);
		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);
	destroy_pointer_tab((void**)config_tab, n);
	destroy_pointer_tab((void**)thread_ids, n);
	return 0;
}
