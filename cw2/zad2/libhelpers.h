#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void close_file(FILE* f);

double get_time(struct timespec* timespec_s, struct timespec* timespec_e);

void log_time(double time, char program_version);