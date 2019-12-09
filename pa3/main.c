/*test machine: csel-kh1250-02.cselabs.umn.edu * date: 11/12/19
* name: Li-Sha Lim, Jamal Khan
* x500: limxx518, khanx090 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

// Globals
NODE head;
NODE *tail;
unsigned int histogram[26] = {0};  // Global histogram that stores alphabet counts
pthread_mutex_t queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t results = PTHREAD_MUTEX_INITIALIZER;
bool endofFile;  // boolean to allow producer and consumer to note when end of file has been reached.
FILE *lg;
unsigned int bufSize = 0;

int main(int argc, char *argv[]) {
	if(argc < 3) {
		printf("Min 3 arguments expected\n");
		return -1;
	}
	
	pthread_t thread_id[atoi(argv[1])+1];
	FILE *result;
	ARG arg[atoi(argv[1])+1];
	//Initialize qSize of arg[0],(producer) to 0
	arg[0].qSize = 0;
	//Initialize all bool values of log to be false initially
	for(int i=0; i<atoi(argv[1])+1; i++)
		arg[i].log = false;
	
	if(argc > 5){
		printf("Max of 5 arguments expected\n");
		return -1;
	}
	else if((strstr(argv[1], "txt") != NULL)){
		printf("Wrong order of arguments entered.\n");
		printf("Format is ./wcs #consumer filename [option] [#queue_size]\n");
		return -1;
	}
	else if(!atoi(argv[1]) || strchr(argv[1], '-')){
		printf("Positive integer value is expected as argument for argv[1]\n");
		return -1;
	}
	else if(atoi(argv[2])){
		printf("Filepath expected for argv[2]\n");
		return -1;
	}
	if(argc > 3 && !strcmp(argv[3], "-pb")){
		printf("Argument -pb not accepted. Formats are: -b, -p or -bp\n");
		return -1;
	}
	if(argc > 3 && strchr(argv[3], 'p')){
		for(int i=0; i<atoi(argv[1])+1; i++)
			arg[i].log = true;
		if((lg = fopen("log.txt", "a")) == NULL){
		perror("Error writing to log file: ");
		exit(-1);
		}
	}
	if(argc > 3 && strchr(argv[3], 'b')){
		if(argc == 4 || !atoi(argv[4])){
			printf("Queue size number expected at argv[4]\n");
			return -1;
		}
		else if(atoi(argv[4]) < 0){
			printf("Negative number for consumers not accepted\n");
			return -1;
		}
		arg[0].qSize = atoi(argv[4]);
	}
			
	//Now fill in the necessary thread data:
	arg[0].fp = argv[2];	
	arg[0].id = -1;			//Producer ID
	for(int i=0; i<atoi(argv[1]); i++)
		arg[i+1].id = i;
	
	//Initialize head to be a dummy head node, and tail to initially point to head
	head.next = NULL;
	tail = &head;
	
	//Create producer thread to feed shared queue buffer lines of words
	pthread_create(&thread_id[0], NULL, producer, (void*)&(arg[0]));
	
	//Create number of consumer threads acccording to input arguments
	for(int i=0; i<atoi(argv[1]); i++)
		pthread_create(&thread_id[i+1], NULL, consumer, (void*)&(arg[i+1]));
	
	//Master thread waits for producer and consumer threads to join
	for(int j=0; j<atoi(argv[1])+1; j++)
		pthread_join(thread_id[j], NULL);		
		
	//Master Finalize, master thread prints histogram results to result.txt
	if((result = fopen("result.txt", "a")) == NULL){
		perror("Error writing to result file: ");
		exit(-1);
	}
	for(int i=0; i<26; i++)
		fprintf(result, "%c: %d\n", i+'a', histogram[i]);
	fclose(result);
	if(arg[0].log ==true)		//Any arbitrary struct arg should work
		fclose(lg);
	printf("Producer and consumer threads have completed execution. Results are: \n");
	for(int i= 0; i< 26; i++)
		printf("%c: %d\n", i+'A', histogram[i]);
	
	return 0;
}