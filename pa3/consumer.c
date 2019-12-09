/*test machine: csel-kh1250-02.cselabs.umn.edu * date: 11/12/19
* name: Li-Sha Lim, Jamal Khan
* x500: limxx518, khanx090 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include <stdbool.h>
//Condition variable for consumers to wait on signal from producer in the event the queue buffer is empty
pthread_cond_t emptyQueue = PTHREAD_COND_INITIALIZER;

void *consumer(void* arg){
	ARG *data = (ARG*)arg;
	NODE *temp;
	int id = ((ARG*)arg)->id;
	//char* string;
	char string[MAX_CHAR];
	char start;
	char *ptr;
	unsigned char state = FINDING_START;
	//unsigned int debug[26] = {0};
	unsigned int lineNo = 0;
	
	while(1){	//forever loop is needed to prevent consumer thread from exiting after processing a package and producer 
				//hasn't reached EOF yet.
					
		//Check if queue is empty, if it is call pthread_wait
		pthread_mutex_lock(&queue);
		printf("Consumer thread %d executing\n", id);
		if(data->log == true)
			fprintf(lg, "consumer %d\n", id);
		while(head.next == NULL){
			//If empty, check if EOF flag is raised. If true terminate thread
			if(endofFile == true){
				printf("Entered, thread %d\n", id);
				if(data->log == true)
					fprintf(lg, "consumer %d: -1\n", id);
				pthread_mutex_unlock(&queue);
				pthread_exit(NULL);
			}
			printf("Empty queue, thread %d will wait on signal\n", id);
			pthread_cond_wait(&emptyQueue, &queue);
			printf("Condition signal received. Thread %d will process new package, if any\n", id);
		}	//Important to extend while loop until here to account for case where there are multiple threads vying for 
			//the last package, or there are more threads than the last few packages in buffer
		
		//If execution reaches here at any point, this signals available packages in queue
		//Consumer gets line number of text file
		printf("Thread %d doing stuff\n", id);
		lineNo = head.next->lineNum;
		bufSize--;
		//Signal to producer that there is space available to add to buffer
		pthread_cond_signal(&fullQueue);
		//Consumer takes a package from head of queue
		//string = head.next->str;		//Can't do this, calling free later on causes string to point to invalid memory
		memset(string, 0, sizeof(string));
		strcpy(string, head.next->str);
		ptr = string;
		//printf("Copied string is %s\n", string);
		//delete node once package is successfully taken
		temp = head.next;
		head.next = head.next->next;
		free(temp);
		if(head.next == NULL)	//reassign tail to head
			tail = &head;
		//printf("head.next->str is %s\n", head.next->str);
		pthread_mutex_unlock(&queue);
		
		/*Process the line of words. It's safe to execute without using a mutex lock since process
		 * is done on a unique package grabbed from the buffer */
		 if(data->log == true)
			fprintf(lg, "consumer %d: %d\n", id, lineNo);
		 while(*ptr != '\0'){	//using comparison to '\n' causes unexpected outputs, not sure why
			switch(state){
				case FINDING_START: if((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z')){
										start = ptr[0];
										state = FINDING_END;
									}
									break;
				case FINDING_END: 	if(!(*ptr >= 'A' && *ptr <= 'Z') && !(*ptr >= 'a'  && *ptr <= 'z')){
										//Synchronize count results with global histogram
										if((start  >= 'A' && start <= 'Z')){
											pthread_mutex_lock(&results);
											histogram[start-'A']++;
											//debug[start-'A']++;
											pthread_mutex_unlock(&results);
										}
										else{
											pthread_mutex_lock(&results);
											histogram[start-'a']++;
											//debug[start-'a']++;
											pthread_mutex_unlock(&results);
										}
										state = FINDING_START;
									}
									break;
			}
			ptr++;
		}
		if(state == FINDING_END){
				if((start  >= 'A' && start <= 'Z')){
					pthread_mutex_lock(&results);
					histogram[start-'A']++;
					//debug[start-'A']++;
					pthread_mutex_unlock(&results);
				}
			else{
				pthread_mutex_lock(&results);
				histogram[start-'a']++;
				//debug[start-'a']++;
				pthread_mutex_unlock(&results);
			}
		}
		/*printf("Executing on line number %d of text file\n", lineNo);
		for(int i = 0; i < 26; i++){
			printf("%c: %d\n", i+'A', debug[i]);
			debug[i] = 0;
		}*/
	}
}
									
				
