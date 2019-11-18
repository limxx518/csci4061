#include "phase4.h"
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
#include <linux/limits.h>	//for the PATH_MAX #define statement

// You are free to use your own logic. The following points are just for getting started
/* Final Result
	1)	The master process reads the "ReducerResult.txt"
	2) 	Print the contents to standard output which is redirected to "FinalResult.txt"
			i.e., on using printf, the statement should be written to "FinalResult.txt"
*/

int phase4(void)
{
	FILE* final;
	FILE* read;
	char line[PATH_MAX];
	
	if((final = fopen("FinalResult.txt", "a")) == NULL)
	{
		perror("Error creating FinalResult.txt: ");
		return -1;
	}
	//use dup2 to redirect stdout to file output
	if(dup2(fileno(final), STDOUT_FILENO) == -1)
	{
		perror("Error redirecting output stream to file stream: ");
		return -1;
	}
	if((read = fopen("ReducerResult.txt", "r")) == NULL)
	{
		perror("Error opening ReducerResult.txt for reading: ");
		return -1;
	}
	while(fgets(line, PATH_MAX, read))
		printf("%s", line);
	fclose(read);
	fclose(final);
	return 0;
}
