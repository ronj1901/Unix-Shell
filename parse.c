#include "parse.h"

/*
** Some little helper functions (could be macros, but functions are safer).
*/
static int isShellSymb(char c) {
    return (c)=='|' || (c)=='<' || (c)=='>';
}
static int isSeparator(char c) {
    return isspace(c) || isShellSymb(c);
}

/*
** Parse: parse a simple nsh command line, and put the results into sc.
** INPUT: char *line, a string containing whitespace-separated tokens.
** OUTPUT: struct commandLine *sc
** NOTE: No syntax checking occurs.  Empty commands are not detected,
**      for example, so the calling function needs to do syntax checking.
** RETURN VALUES: 0 on success.  (Currently it always returns 0.)
*/

//helper function to handle cd command
int cd(char *path) {
    return chdir(path);
}


void execute(struct commandLine *sc){
    int tmpin = dup(0);
    int tempout  = dup(1);
    int status;
    //set the initial input
    int fdin;
    if (sc->infile){
        fdin = open(sc->infile, O_RDONLY);
        if (fdin < 0){
            perror("could not open an input file");
            exit(0);
        }
    }
    else {
          // use default input
        fdin = dup(tmpin);
    }

    
    int fdout;
    int i;
    
    for (i =0; i < sc->numCommands; i++){
        dup2(fdin, 0); // need to do error handling
        close(fdin);
        // setup output
        if ( i == sc->numCommands - 1 ){
            // last command 
            if (strcmp(sc->argv[0], "cd") == 0){
                if (cd(sc->argv[1]) < 0) {
                perror(sc->argv[1]);
            }

                    /* Skip the fork */
                    continue;
            }

            if (sc->outfile){

                if(sc->append == '1'){
                    fdout = open(sc->outfile, O_WRONLY | O_APPEND, 0600);
                }
                else{
                    fdout = open(sc->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                }
                
                if (fdout < 0){
                    perror("could not open an output file");
                    exit(0);
                }

            }
            else{
                // use default output
                fdout  = dup(tempout);
            }
        }
        else{
            //Not last comand, so create a pipe
            int fdpipe[2];
            pipe(fdpipe);
            fdout  = fdpipe[1];
            fdin = fdpipe[0];

        } // end if else
        //redirect output
        dup2(fdout, 1);
        close(fdout);

        //create child process
        int ret  = fork();
        if ( ret == 0){
            // inside child process
            execvp(sc->argv[sc->cmdStart[i]], &(sc->argv[sc->cmdStart[i]]));
            perror("execvp");
            _exit(1);
        }
        else if ( ret > 0)
        // I am in the parent process, so wait for all children to finish 
            wait(&status);
        else{
            // fork was not successful
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
             

    }  // end of for loop

    //restore the in out defaults
    dup2(tmpin,0);
    dup2(tempout, 1);
    close(tmpin);
    close(tempout);

   // wait for the last command
   wait(&status);

}


int Parse(char *line, struct commandLine *sc)
{
    int argc = 0;

    assert(strlen(line) < MAX_LINE);
    strcpy(sc->buf, line);
    line = sc->buf;

    sc->infile = sc->outfile = sc->argv[0] = NULL;
    sc->append = sc->numCommands = 0;
    sc->cmdStart[0] = 0;    /* the 0th command starts at argv[0] */
    
    /*
    ** This is the main loop, reading arguments into argv[].
    ** We check against MAX_ARGS-1 since the last one must be NULL.
    */
    while(argc < MAX_ARGS-1 && sc->numCommands < MAX_COMMANDS)
    {
        /*
        ** At this point, "line" is pointing just beyond the end of the
        ** previous argument (or the very first character if this is
        ** the first time through the loop), which should be a whitespace
        ** character.
        */

        while(isspace(*line)) /* skip initial whitespace */
            ++line;
        if(*line == '\0')   /* end-of-line, we're done */
            break;

        /*
        ** At this point, "line" is pointing at a non-space character; it
        ** may be the start of an argument, or it may be a special character.
        */
        
        switch(*line)
        {
        case '|':
            /* terminate the previous argument and bump to next character */
            *line++ = '\0';
            sc->argv[argc++] = NULL;    /* terminate the command */
            /* the next command will start at this position */
            sc->cmdStart[++sc->numCommands] = argc;
            continue;   /* go back to the top of the while loop */

        case '<':
            *line++ = '\0'; /* terminate argument and go to next character */
            while(isspace(*line))   /* skip whitespace */
                ++line;
            sc->infile = line;
            break;

        case '>':
 	    if(*line == '>') /* second '>' means append */
	    {
		   sc->append = 1;
		   line++;
	    }
            while(isspace(*line))   /* skip whitespace */
                ++line;
            sc->outfile = line;
            break;

        /*
        ** Here would be a good place to check for other special characters
        ** (quotes, '&', etc) and do something clever.
        */

        default:    /* it's just a regular argument word */
            sc->argv[argc++] = line;
            break;
        }
        
        while(*line && !isSeparator(*line))  /* find the end of the argument */
            ++line;
        if(*line == '\0')        /* end-of-line, we're done */
            break;
	else if(isShellSymb(*line))
	    continue;	// don't increment line pointer so we'll see the ShallSymb on next loop iteration.
        else
            *line++ = '\0'; /* terminate the argument and go to next char */
    }

    /* We are at the very end of "line" */
    sc->argv[argc] = NULL;  /* terminate the last command */
    sc->argv[argc + 1] = NULL;  /* marker meaning "no more commands" */

    /*
    ** There's a nasty special case to handle here.  If the command line
    ** contains any commands, then numCommands can simply be set to
    ** (number of '|' symbols + 1).  Note that the only place above
    ** that numCommands is increased is in the "case '|'" of the switch.
    ** So we should add 1 here.  BUT, if there were NO commands (ie, the
    ** command line was empty), then we should NOT add 1.
    */
    if(argc > 0)
        ++sc->numCommands;

    return 0;
}
