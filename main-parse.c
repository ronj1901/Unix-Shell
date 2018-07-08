#include "parse.h"

/*
** A very simple main program that re-prints the line after it's been scanned and parsed.
*/
int main(int argc, char *argv[])
{

	// read a file when it is passes as an argument

    FILE *input;
    char line[MAX_LINE];

    pid_t child_pid;
	int stat_loc;

	int in =0; 
	int out  = 0;
	int current_out, current_in;

    if(argc == 2)
    {   
    	size_t len = 0;
		ssize_t read;
		input = fopen(argv[1], "r");
		if(input == NULL)
		{
		    perror(argv[1]);
		    exit(1);
		}

		// we dont have error reading file, so we gonna process each line now

		while (fgets(line, sizeof(line), input) != NULL ) {
			
			struct commandLine cmdLine;

			if(line[strlen(line)-1] == '\n'){
			    line[strlen(line)-1] = '\0';   /* zap the newline */
			}

			Parse(line, &cmdLine);
			execute(&cmdLine);

		}
		 
		fclose(input);
		exit(EXIT_SUCCESS);

		//end
    }
    else
    {
		assert(argc == 1);
		input = stdin;
		printf("nsh> ");

	fflush(stdout);
    }

    setlinebuf(input);
    while(fgets(line, sizeof(line), input))
    {
		int i;
		
		struct commandLine cmdLine;

		if(line[strlen(line)-1] == '\n')
		    line[strlen(line)-1] = '\0';   /* zap the newline */

		Parse(line, &cmdLine);
		execute(&cmdLine);
	

        if(input == stdin)
        {
            printf("nsh> ");
            fflush(stdout);
        }

    }

    return 0;
}
