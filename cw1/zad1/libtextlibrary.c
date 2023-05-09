#include "libtextlibrary.h"

char command[COMMAND_MAX_SIZE];

Counter createCounter(int size){
	Counter counter;
	counter.size = size;
	counter.currSize = 0;
	counter.tab = (char**)calloc(size, sizeof(char*));
	return counter;
}

void count(Counter* counter, char* filename){

	char tmpFilename[] = "/tmp/tmpfile_XXXXXX";
	int file = mkstemp(tmpFilename);

	if(file == 0){
		fprintf(stderr, "Failed to create temp file");
		return;
	}

	snprintf(command, COMMAND_MAX_SIZE, "wc '%s' 1> '%s' 2>/dev/null",filename, tmpFilename);
	system(command);

	FILE *fp = fopen(tmpFilename, "r");
	fseek(fp, 0L, SEEK_END);
	int fileSize = ftell(fp);
	rewind(fp);
	char* fileContent = calloc(fileSize, sizeof(char));
	fgets(fileContent, fileSize, fp);
	fclose(fp);

	snprintf(command, COMMAND_MAX_SIZE, "rm -f '%s' 2>/dev/null", tmpFilename);
	system(command);

	if(!strlen(fileContent)){
		fprintf(stderr, "Failed to read temp file");
		return;
	}

	if(counter->currSize >= counter->size){
		fprintf(stderr, "Counter is full, unable to save data");
		return;
	}
	counter->tab[counter->currSize] = fileContent;
	counter->currSize++;
}

char* getBlock(Counter *counter, int index){

	if (index < 0 || index > counter->currSize ){
		fprintf(stderr, "Invalid index");
		return "";
	}
	if (!counter->tab[index]){
		fprintf(stderr, "Empty block at index %d\n", index);
		return "";
	}

	return counter->tab[index];

}

void clearBlock(char* block){
	free(block);
	block = 0;
}

void delBlock(Counter *counter, int index){
	if(counter->tab[index]){
		free(counter->tab[index]);
		counter->tab[index] = 0;
	}else{
		fprintf(stderr, "Desired block already empty\n");
	}
}

void destroyCounter(Counter* counter){
	for(int i = 0; i < counter->currSize; i++){
		if(counter->tab[i]){
			clearBlock(counter->tab[i]);
		}
	}
	free(counter->tab);
}
