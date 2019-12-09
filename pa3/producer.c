/*test machine: csel-kh1250-02.cselabs.umn.edu * date: 11/12/19
* name: Li-Sha Lim, Jamal Khan
* x500: limxx518, khanx090 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

/*NOTES: For mutex + condition to avoid busy-wait, producer calls signal function upon adding a node
 * 		 Conversely, consumer function calls wait function in the event that head.next = NULL, signalling empty 
 * 		 queue.
 */

//Condition variable for producer to wait on signal from consumers in the event it's buffer is full
pthread_cond_t fullQueue = PTHREAD_COND_INITIALIZER;

void *producer(void *arg){
	ARG* data = (ARG*)arg;
	char *filename = ((ARG*)arg)->fp;
	FILE *fptr;
	char line[MAX_CHAR];
	unsigned int lineNo;
	
	if(data->log == true)
		fprintf(lg, "producer\n");
	
	if((fptr = fopen(filename, "r")) == NULL){
		printf("Error opening file %s\n", filename);
		perror("Failed to open file: ");
		exit(-1);
	}
	//Get each line of words in text file to pass as a package to buffer
	while(fgets(line, MAX_CHAR, fptr) != NULL){
		//lock critical section to add to tail of queue
		pthread_mutex_lock(&queue);		
		lineNo++;
		if(data->log == true)		//Safe to assume log FILE pointer remains open?
			fprintf(lg, "producer: %d\n", lineNo);
		if(data->qSize != 0 && bufSize == data->qSize){
			printf("Buffer size full. Producer will wait on signal from consumers\n");
			pthread_cond_wait(&fullQueue, &queue);
			printf("Condition signal received. Producer will add to queue\n");
		}
		tail->next = (NODE*)malloc(sizeof(NODE));
		bufSize++;	
		strcpy(tail->next->str, line);
		
		///This bug was a pain to fix, somehow pointer reassignment doesn't work, and reason why not figured out yet.
		///head.next->str always gets updated to the latest string addition, replicating addition at head node
		///BUT...address of node additions is logical, so logic for node addition at tail is sound.
		
		///DEBUG:
		/*printf("Address of head is %ld\n", &head);
		 * printf("Address of head.next is %ld\n", head->next);
		 * printf("Address of tail is %ls\n", tail);
		 */
		 ///PROBLEM STATEMENT:
		//tail->next->str = line;	
		
		//printf("Package copied is %s\n", tail->next->str);
		//Update line number for that node
		tail->next->lineNum = lineNo;
		tail->next->next = NULL;
		tail = tail->next;
		//Call broadcast function to alert consumers of available packages, broadcast because there are several consumers
		pthread_cond_broadcast(&emptyQueue);
		printf("Producer has put a new package in queue\n");
		//printf("Producer now unlocking mutex\n");
		pthread_mutex_unlock(&queue);
	}
	//EOF file reached	
	fclose(fptr);
	pthread_mutex_lock(&queue);		
	printf("Producer has reached EOF. Terminating its thread...\n");
	//Set endofFile flag to true to send notifications to all consumers specifying there'll be no more data
	endofFile = true;
	if(data->log == true)
		fprintf(lg, "producer: -1\n");
	pthread_cond_broadcast(&emptyQueue);
	pthread_mutex_unlock(&queue);
	pthread_exit(NULL);
}