#define _DEFAULT_SOURCE
#define NUM_ARGS 0
#define NUM_THRD 1000

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

// DO NOT ALTER!
struct philosopher {

	int food;
	pthread_mutex_t* left;
	pthread_mutex_t* right;
};

// DO NOT ALTER!
void eat(struct philosopher* p) {

	--(p->food);
	
	usleep(rand() % 10);
}

void dine(struct philosopher* p) {

	// Grab the utensils.
	int left, right;
	
	while(1){
		pthread_mutex_lock(p->left);
        usleep(rand() % 10); //NOTE: This small sleep exists to heighten the
                             //      likelihood of the original implementation
                             //      encountering deadlock. Removing this wait
                             //      is not a correct solution to the problem
                             //      of deadlock in this program, though its
                             //      removal may allow the program to terminate
                             //      more frequently. A complete solution
                             //      ensures that no deadlock is possible.
		right = pthread_mutex_trylock(p->right);
		if(!right){
			printf("Eating\n");
			eat(p);
			// Put them down.
			pthread_mutex_unlock(p->right);
			pthread_mutex_unlock(p->left);
			break;
		}
		else if(!left)
			pthread_mutex_unlock(p->left);
		}
			
	
	/*
	//printf("Finally eating\n");
	eat(p);
	
	// Put them down.
	pthread_mutex_unlock(p->right);
	pthread_mutex_unlock(p->left);
	//printf("End\n");*/
}

// DO NOT ALTER!
void*  threadFun(void* arg) {

	usleep(rand() % 10);

	struct philosopher* p = (struct philosopher*) arg;
	
	while (p->food > 0) {
		printf("Food number is %d\n", p->food);
		dine(p);
	}
        return NULL;
}

// DO NOT ALTER!
int main(int argc, char** argv) {

	if (argc != NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Seed the random generator.
	srand(time(NULL));

	// Create threads.
	pthread_t pool[NUM_THRD];
	struct philosopher* p[NUM_THRD];
	
	// Create the utensils (mutexes).
	pthread_mutex_t* locks[NUM_THRD];
	for (int i=0; i < NUM_THRD; ++i) {
	
		locks[i] = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(locks[i], NULL);
	}
	
	// Create the philosophers.
	for (int i=0; i < NUM_THRD; ++i) {
	
		p[i] = (struct philosopher*) malloc(sizeof(struct philosopher));
		p[i]->food = 100;
		p[i]->left = locks[i];
		p[i]->right = locks[(i+1) % NUM_THRD];
	}

	clock_t start;
	clock_t end;

	start = clock();

	for (int i=0; i < NUM_THRD; ++i) pthread_create(&pool[i], NULL, threadFun, (void*) p[i]);
	for (int i=0; i < NUM_THRD; ++i) pthread_join(pool[i], NULL);

	end = clock();
	
	printf("Time (in us) to complete = %f\n", (double) (end - start));
}
