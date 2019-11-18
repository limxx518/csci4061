/*test machine: csel-kh1250-22.cselabs.umn.edu
 * date: 10/25/19
 * name: Jamal Khan, Li-Sha Lim
 * x500: khanx090, limxx518
 * */
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	// argument count check
	if(argc != 3)
	{
		printf("Error: Only 3 arguments expected\n");
		return -1;
	}		
	//just make a function call to code in phase1.c
	//phase1 - Data Partition Phase
	if(phase1(argv[1], atoi(argv[2])) == -1)
		return -1;	
	//just make a function call to code in phase2_3.c
	//phase2_3 - Map and Reduce Function
	if(phase2_3(atoi(argv[2])) == -1)
		return -1;	
	if(phase4() == -1)
		return -1;	
	return 0;

}
