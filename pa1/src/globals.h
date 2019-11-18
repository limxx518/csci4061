
#define LINE_SIZE 128
#define MAX_LINES 128
#define MAX_DEP 8
//Extra credit
#define MAX_RECIPES_PT 8
#define MAX_TARGETS 128
#define MAX_PARM 32

typedef struct target_block
{
	char *name;
	char *depend[MAX_DEP];
	char *recipe[MAX_RECIPES_PT];
	unsigned int dep_count;
	unsigned int recipe_count;
	unsigned char visited;
	unsigned int parallel_ct[MAX_RECIPES_PT];
	unsigned int parallel_ct_tail;				//record last index/string of particular target
} target;

//Global variables
extern char lines[MAX_LINES][LINE_SIZE];
extern int i;							//index for lines array, global scope because process_file and parse_lines functions use it
extern target targets[MAX_TARGETS];
extern int visited[MAX_TARGETS];
extern int M[MAX_TARGETS][MAX_TARGETS];	//adjacency matrix
extern char* ordered_recipes[MAX_TARGETS*MAX_RECIPES_PT];
extern unsigned int ordered_recipes_offset[MAX_RECIPES_PT*MAX_TARGETS];
extern int r_index;					//this needs to be global scope for value to persist out of the recursive function call
extern int rCount;						//similar as above
extern char* args[LINE_SIZE];
extern char* args2[MAX_RECIPES_PT][LINE_SIZE];

//Function declarations
int process_file(char *fname);
int parse_lines(void);
void tokenize_arguments(char* str, int num, int boolean);
void create_adjacency_matrix(int num_targets);
void DFS(int pt, int numTargets);
