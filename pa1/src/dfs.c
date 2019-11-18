#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "globals.h"

int visited[MAX_TARGETS];
int M[MAX_TARGETS][MAX_TARGETS];
char* ordered_recipes[MAX_TARGETS*MAX_RECIPES_PT];
unsigned int ordered_recipes_offset[MAX_RECIPES_PT*MAX_TARGETS];
int r_index = 0;
int rCount = 0;

void create_adjacency_matrix(int num_targets)
{
	for(int a=0; a<num_targets; a++)
	{
		for(int b=0; b<targets[a].dep_count; b++)
		{
			for(int c=0; c<num_targets; c++)
			{
				if(!strcmp(targets[a].depend[b], targets[c].name))
					M[a][c] = 1;
			}
		}
	}
	//Debug purposes only
	/*for(int a=0; a<num_targets; a++)
	{
		for(int b=0; b<num_targets; b++)
			printf("%d\t", M[a][b]);
		printf("\n");
	}*/
}

void DFS(int pt, int numTargets)
{
	int j;
	int a = 0;
	int r_cp = 0;
	
	visited[pt] = 1;
	//printf(" %d->", pt);
	for(j=0; j<numTargets; j++)
	{
		if(M[pt][j] && !visited[j])
			DFS(j, numTargets);
	}
	if(!targets[pt].visited)
	{
		r_cp = r_index;
		//Strategy to handle parallel execution is by going through each recipe line for a particular target,
		//up to its last as stored in parallel_ct_tail; for each line check if the parallel_ct for that recipe
		//line is non-zero. Parallel_ct was determined in the parse_lines function. If non-zero, this denotes 
		//parallel processing of recipes is necessary. Since the ordered recipes have been pre-parsed with recipes 
		//separated by commas extracted to separate recipes, approach is to store an offset for the particular
		//index in the ordered_recipes array for the first recipe in a recipe line containing multiple recipes
		//separated by the comma operator. Thus, later in the fork function, function just checks if ordered_recipes_offset
		//is non-zero at that particular index. If non-zero, it tokenizes the recipes in a 2D array and processes them in parallel
		//Then, it updates the index for ordered_recipes by the value of the offset.
		while(a < targets[pt].parallel_ct_tail)
		{
			if(targets[pt].parallel_ct[a])	
			{
				ordered_recipes_offset[r_cp] = targets[pt].parallel_ct[a];
				r_cp += targets[pt].parallel_ct[a];
			}
			else
				r_cp++;
			a++;
		}		
		for(a=0; a<targets[pt].recipe_count; a++)
		{
			ordered_recipes[r_index++] = targets[pt].recipe[a];
			rCount++;				
		}
	}
	targets[pt].visited = 1;	//Mark as visited to avoid duplicate printing for case where there are similar dependencies 
								//among targets.
}
