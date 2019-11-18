#ifndef PHASE2_3_H
#define PHASE2_3_H

//Function prototypes for mapper and reduce processes
int mapper(char *str, int mapper_id, int fd[][2]);	//Function used by each mapper process to count number of letters corresponding to the words in the text files
int phase2_3(int num_fork);	//Wrapper for mapper function

#endif
