#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define COMMAND_MAX_SIZE 2048

typedef struct {
	char** tab;
	int size;
	int currSize; 
} Counter;

Counter createCounter(int size);
void count(Counter* counter, char* file);
char* getBlock(Counter* counter, int index);
void delBlock(Counter* counter, int index);
void destroyCounter(Counter* counter);
