#include "libhelpers.h"

void close_file(FILE* f){
    fclose(f);
}

double get_time(struct timespec* timespec_s, struct timespec* timespec_e){
    double realtime = ((double) timespec_e->tv_sec - timespec_s->tv_sec);
	double nano =  ((double) timespec_e->tv_nsec - timespec_s->tv_nsec) / (double)1000000000;
    return nano + realtime;
}

void log_time(double time, char program_version){
    FILE* raport = fopen("pomiar_zad_2.txt", "a");    
    char* tmp = calloc(256, sizeof(char));
    snprintf(tmp, 256, "reverse%c: %lfs\n",program_version,time);
    fwrite(tmp, sizeof(char), strlen(tmp), raport);
    fclose(raport);
}