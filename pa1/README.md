/*test machine: atlas.cselabs.umn.edu
* date: 10/8/19
* name: Jamal Khan, Li-Sha Lim
* x500: khanx090, limxx518
*/

## The Purpose of the Program / What the Program Does
The program has a method called process_file() that reads the contents of a makefile, storing it into a two-dimensional char array. Then, parse_lines() parses the data entered into the 2D char array into a struct that will store each target in the makefile and its associated dependencies, recipes, and their respective counts. 

Once we have all our targets stored in a struct array, we can either print out all the targets with the -p flag, or determine the correct order to execute all the recipes in the makefile based on target dependencies using a recursive depth first search function (dfs()) using the -r flag. Lastly, the program uses fork() and execvp() to execute the recipes in the correct order using child process(es) while the parent waits.

## How to Compile & Run the Program
In your terminal, navigate to the pa1 directory and use the command ```make``` to compile the program. This will create an executable file called mymake. To run mymake, there are four different ways to execute it:

```shell
./mymake filename\n");
./mymake filename [target]\n");
./mymake [-p] filename\n");
./mymake [-r] filename\n");
```

Up to two command line arguments can be specified. The filename argument is mandatory, and must point to a properly formatted makefile. The optional -p and -r flags must be in the correct positions are shown above. The -p flag prints out the makefile, listing all the targets and their corresponding dependencies and recipes. The -r flag prints out the recipes in the order in which they need to be executed based on their dependencies.

## Assumptions outside what is mentioned in the assignment pdf
- Regarding parsing makefiles: The next line immediately after a target that has a tab BUT NOT a colon is a recipe. This therefore assumes that a recipe cannot contain a colon.
- Recipes must not contain commas unless it is to designate seperation from another recipe.
- Targets must have unique names.
- Makefiles are properly formatted as described in assignment handout section 1.1. i.e. "Targets and dependencies are separated by a colon (:). Multiple dependencies for
a target are separated by space. Each recipe for a target must be indented via a single tab (\t,
not 2/3/4/8 spaces). Recipes within the same target, must be executed in the order they appear."
- Commmand line arguments are case sensitive.
- Target is not an empty string.

## Team names and x500
Team name: team0

x500s: khanx090, limxx518

## Your and your partners individual contributions
We both did the project seperately so as to adequately learn the coursework, but we collaborated as we went along, discussing the challenges involved. Around the time we both finished part 1.3, we came together and combined the best bits and ideas of both of our programs together for submission. But both instances of our programs passed all the tests. So we each completed every task required of the assignment.

## If you have attempted extra credit
Yes, including but not limited to:
- Multiple recipes per line with comma as a seperator.

- Parsing recipe lines with commas in such a way so as to remove space(s) between the comma and the next recipe. 

- Parallel execution functionality.