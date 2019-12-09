/*test machine: csel-kh1250-02.cselabs.umn.edu * date: 11/12/19
* name: Li-Sha Lim, Jamal Khan
* x500: limxx518, khanx090 */

/*
header.h, header for all source files
it will: 
- structure definition
- global variable, lock declaration (extern)
- function declarations
*/

#ifndef _HEADER_H_
#define _HEADER_H_

//Include statements
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>

//Define statements
#define MAX_CHAR 1024
#define FINDING_START 0
#define FINDING_END 1

//Structure definitions
//Linked list node
typedef struct NODE{
	char str[MAX_CHAR];
	unsigned int lineNum;
	struct NODE* next;
}NODE;
//Data struct for passing to producer/consumers
typedef struct ARG{
	char *fp;
	int id;
	bool log;
	unsigned int qSize;
}ARG;

//Global variables
extern NODE head;		//Dummy head node
extern NODE* tail;		//Tail node pointer for insertion of new packages
extern unsigned int histogram[26];
extern pthread_mutex_t queue;
extern pthread_mutex_t results;
extern pthread_cond_t emptyQueue;
extern pthread_cond_t fullQueue;
extern bool endofFile;
extern FILE *lg;
extern unsigned int bufSize;

//Function declarations
/* Producer processes the text file by taking each line and placing it as a package
in the shared queue buffer with consumers */
void *producer(void *arg);

/* Consumer continuously grabs an available package from the queue buffer, counts
the number of times each letter occurs at the start of a word, and updates the count
by synchronizing the count with a global histogram that is updated by all consumers. */
void *consumer(void* arg);
#endif