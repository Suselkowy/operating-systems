#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdbool.h>
#include "libtextlibrary.h"


Counter* counter;

void printTimes(struct timespec* timespec_s, struct timespec* timespec_e, struct tms* tms_s, struct tms* tms_e);
int sumInput(char* text, int startIndex);
void CLILogic(char* buff, bool* initialized, bool* error, int* finished, Counter* counter); 

int main(){

	#ifdef DYNAMIC
	#include <dlfcn.h>
	void* lib_handle = dlopen( "libtextlibrary.so", RTLD_LAZY);

	if(lib_handle == NULL){
		fprintf(stderr, "lib not working\n");
		return -1;
	}

	createCounter = dlsym(lib_handle, "createCounter");
    count = dlsym(lib_handle, "count");
    getBlock = dlsym(lib_handle, "getBlock");
    delBlock = dlsym(lib_handle, "delBlock");
    destroyCounter = dlsym(lib_handle, "destroyCounter");
	#endif

	int finished = 0;

	//Counter counter;
	counter = malloc(sizeof(Counter));
	bool initialized = false;

	struct timespec start, end;
	struct tms t_s, t_e;

	while(!finished){

		char* buff = NULL;
		size_t buff_size;
		getline(&buff, &buff_size, stdin);
		bool error = false;
		
		//timer start
		times(&t_s);
		clock_gettime(CLOCK_REALTIME, &start);

		CLILogic(buff, &initialized, &error, &finished, counter);

		clock_gettime(CLOCK_REALTIME, &end);
		times(&t_e);
		
		free(buff);
		if(!error){
			printTimes(&start, &end, &t_s, &t_e);
		}
	}

	if(initialized){
		destroyCounter(counter);
		free(counter);
	}

	return 0;

}

void printTimes(struct timespec* timespec_s, struct timespec* timespec_e, struct tms* tms_s, struct tms* tms_e){
	int ticks = sysconf(_SC_CLK_TCK);
	double realtime = ((double) timespec_e->tv_sec - timespec_s->tv_sec);
	double nano =  ((double) timespec_e->tv_nsec - timespec_s->tv_nsec) / (double)1000000000;
	double usertime = (double)(tms_e->tms_utime - tms_s->tms_utime) / ticks;
	double systemtime = (double)(tms_e->tms_stime - tms_s->tms_stime) / ticks;
	printf("real time: %lfs\nsystem time: %lfs\nuser time: %lfs\n", realtime+nano, systemtime, usertime);
}

int sumInput(char* text, int startIndex){
	int ans = 0;
	while(text[startIndex] && text[startIndex] >= 48 && text[startIndex]<= 57){
		ans *= 10;
		ans += text[startIndex] - '0';
		startIndex++;
	}
	return ans;
}

void CLILogic(char* buff, bool* initialized, bool* error, int* finished, Counter* counter){
	if(strncmp(buff, "init", 3) == 0){
		if(*initialized){
			fprintf(stderr, "Destroy previous object first\n");
			*error = true;
		}else{
			int s = sumInput(buff, 5);
			if(s <= 0){
				fprintf(stderr, "Invalid length\n");
				*error = true;
			}else{
				Counter tmp = createCounter(s);
				counter->tab = tmp.tab;
				counter->size = tmp.size;
				counter->currSize = tmp.currSize;
				tmp.tab = 0;
				*initialized = true;
			}
		}	
	}else if(strncmp(buff, "exit", 4) == 0){
		*finished = 1;
		//only for conveniece sake
		*error = true;
	}else if(!*initialized){
		fprintf(stderr, "You should initialize counter first\n");
		*error = true;
	}else if(strncmp(buff,"count",5)==0){
		char* ptr = strtok(&buff[6], "\n");
		count(counter, ptr);
	}
	else if(strncmp(buff,"show",4)==0){
		int s = sumInput(buff, 5);
		printf("%s\n", getBlock(counter, s));
	}
	else if(strncmp(buff, "delete", 6)==0){
		int s = sumInput(buff, 7);
		delBlock(counter, s);
	}
	else if(strncmp(buff, "destroy", 7)==0){
		destroyCounter(counter);
		*initialized = false;
	}
	else{
	fprintf(stderr, "Invalid command\n");
		*error = true;
	}
}

