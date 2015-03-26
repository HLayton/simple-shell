/* ACE 4 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>

/* The name of our shell! */
#define SHELLNAME "HermitTheKermit"
/* The number of elements in an array */
#define LEN(array) sizeof(array)/sizeof(array[0])
/* Are two strings equal? */
#define EQ(str1,str2) (!strcmp(str1,str2))
#define UNCHANGED 0
#define MAXIN 512 /* MAXIN is the maximum number of input characters */
#define DELIM " \n\t|><&;" /* DELIM is the string containing all delimiters for tokens */
#define SZ_ARGV 50 /* Size of the argv array */
#define HISTFILE ".hist_list"

/*Keeps count of the command number being executed*/ 
static int count;

/*A structure for storing the command number and string*/
typedef struct {
  int cmd_no;
  char input_line[MAXIN];
} history_line_t;

typedef struct {
    char name[MAXIN];
    char *command[10];
} alias_t;

/* An array for storing history*/
static history_line_t saved_history [20];
static alias_t alias[10];

const char *pathValue;


void save_history() { 
  	FILE *out;
	int i; 
	chdir(getenv("HOME"));

  if ((out = fopen(HISTFILE, "w")) == NULL)
    perror(HISTFILE);

  if (out == NULL)
    return;

  for (i = 1; i <= LEN(saved_history); i++) { 

		if(saved_history[i%20].cmd_no == 0)
			break;
	
		fprintf(out,"%d\n", saved_history[i].cmd_no);
		fprintf(out,"%s\n", saved_history[i].input_line);
			
  }
	

  fclose(out); 
}

void load_history(){

	int i;
	int maxCount=0;
	char input[MAXIN];

	printf("test\n");
	FILE* in = fopen(HISTFILE, "rw");

	if (in == NULL) {
  		printf( "Can't open input file in.list!\n");
  		return;
	}	
	while (!feof(in)) {		
		fgets(input, MAXIN, in);
		strtok(input, "\n");
		i = atoi(input);
		maxCount = (maxCount> i)? maxCount : i;
		saved_history[i%20].cmd_no = i;
		fgets(saved_history[i%20].input_line, MAXIN, in );
		strtok(saved_history[i%20].input_line, "\n");
				
	}
	count = maxCount; // TO CHANGE - needs to find the max number of instruction
	
	fclose(in);
}

void quit() {
	setenv("PATH", pathValue, 1);
	printf("%s\n",getenv("PATH"));
	save_history();
	exit(0);
}

void cd(char **argv) {
	if(argv[2] == NULL){
	  	if (!argv[1]){
	    		chdir(getenv("HOME"));
		}else{
			if(chdir(argv[1]))
				perror(argv[1]);
		}
	}else{
		printf("Too many parameters. Usage: cd [directory]\n");
	}

  
}

/* Return the PATH environment variable */
void getpath(char **argv) {
	if(argv[1] == NULL){
		printf("%s\n", getenv("PATH"));
	}else{
		printf("Expects no parameters. Usage: getpath\n");
	}
}

void setpath(char **argv) {
	if(argv[2] == NULL) {
		if (argv[1] != NULL) {
			setenv("PATH", argv[1], 1);
		} else {
			printf("Setpath takes 1 parameter. Usage: setpath path\n");
		}
	}else{
		printf("Too many parameters. Usage: setpath path\n");
	}
}

void init_alias() {
    int i;
    for (i=0; i<10; i++)
        *(alias[i].command) = calloc(10,sizeof(char*));
}

void setAlias(char **argv)
{
    static int aliasCount;
    int i;
    
    strcpy(alias[aliasCount].name, argv[1]);
    
    *alias[aliasCount].command = argv[2];
    
    printf("%s\n", alias[aliasCount].command);
    
    aliasCount++;
}

int checkAlias(char *name) {
    
    int i;
    
    for(i = 0; i < 10; i++)
    {
        
        if(strcmp(name, alias[i].name) == 0)
        {
            /*printf("This is an alias. Returning index.\n");*/
            return i;
        }
        
        if(NULL == alias[i].name)
        {
            printf("Not an alias\n");
            return -1;
        }
    }
    
    return -1;
}

void printAlias()
{
    int i;
    
    for(i = 0; i < 2; i++)
    {
        printf("Alias name: %s\n", alias[i].name);
        printf("Alias command %s\n", alias[i].command);
    }
}

/*
This instruction is called when the first character of the input is a '!'
meaning a command is invoked from history
*/

char *command_history(char *input, int count) {
	int cmd;

/* '-' means counting backwards from the last commands entered */ 
	if ('-' == input[1]) { 
		cmd = strtoul((input+2), NULL, 10);
		cmd = count - cmd;
		if(cmd <= 0){
			printf("History item does not exist\n");
			return NULL;
		}
	}else {
		cmd = strtoul((input+1), NULL, 10); 
		if(cmd == 0){
			printf("History item does not exist\n");
			return NULL;
		}
	}

	if (cmd < 0 || cmd > count || cmd < count - 20){
			printf("History item does not exist\n");
			return NULL;
	}
	
	return saved_history[cmd%20].input_line;
}

/*
this is a builtin command that just prints the user command history. Commands invoked 
from history or the invocations !## are not saved. It is not printing the commands 
in order of invocation atm i.e. cmd_no order in struct - to be fixed. 
*/  
void history(){  
	int c, d = 1; 
	if(count > 20)
		d = count + 1;

	/* history is saved in array starting at 1 */
	for(c = d; c<(count+21); c++){

		if(saved_history[c%20].cmd_no == 0)
			break;

		printf(" %d  %s\n", saved_history[c%20].cmd_no, saved_history[c%20].input_line);
		
	}
}

char *get_input() {
	
	static char input[MAXIN]; /* declared as static so it's not on the stack */
	char *cwd = get_current_dir_name();
	bool too_much_input = true;
	int i;

	do {
		printf("[%s]%% ", cwd);
		/*Exits on ctrl+D*/
		if (fgets(input, MAXIN, stdin) == NULL) /* get user input */
			quit(); /*Exit on null pointer, given by fgets()*/
		
	}
	/* fgets as scanf() can't handle blank lines */
	/* check if it was a blank line, i.e. just a '\n' input...*/
	while ('\n' == input[0]); /*check if input is valid - i.e. not blank*/

	free(cwd);

	/* Clear the rest of the line if it was longer than the input array */
	for (i = 0; i < MAXIN && input[i] != '\0'; i++) {
		switch (input[i]) {
		case '\n': too_much_input = false; break;
		}
	}

	if (too_much_input) {
		while (getchar() != '\n');
	}
	
	/*Command history*/
	/* if (!strcspn(input, "!"));  ASK if we need to use it  - he recommends it, but we managed without it I think.. */
	
	if (!strcspn(input, "!")) {	
		return command_history(input, count); //count +1
	}

	count++;
	/*after the count++, the history starts being saved at index 1 */
	saved_history[count%20].cmd_no = count;
	input[strcspn(input, "\n")] = 0;
	strcpy(saved_history[count%20].input_line, input);
	 

	/* If we get to this point it there has to be input so just return it. */
	return (input);
}

void tokenise(char *line, char **tokens) {
	int p;
	char *token;

	p = 0;
	tokens[2] = NULL;
	token = strtok(line, DELIM); /* initial strtok call */
	/* While there are more tokens and our array isn't full */
	while (token && (p < SZ_ARGV - 1)) {
		tokens[p++] = token;
		token = strtok(NULL, DELIM); /* ...grab the next token */
	}
	tokens[p] = 0;
}

int internal_command(char **argv) {
    /* Internal commands */
    /* TODO: internal commands as another function */
    /* exit*/
    if (EQ(argv[0], "exit")) {
        quit();
    } else if (EQ(argv[0], "cd")) {
        cd(argv);
        return 0;
    } else if (EQ(argv[0], "getpath")) {
        getpath(argv);
        return 0;
    } else if (EQ(argv[0], "setpath")) {
        setpath(argv);
        return 0;
    } else if (EQ(argv[0], "history")) {
        history();
        return 0;
    } else if (EQ(argv[0], "alias")) {
        setAlias(argv);
        return 0;
    } else if (EQ(argv[0], "printa")) {
        printAlias();
        return 0;
    }
    
    /* Return negative number if command not found */
    return -1;
}

void external_command(char **argv) {
	pid_t pid;

	/* fork a child process */
	pid = fork();
	if (pid < 0) {
		/* error occurred */
		fprintf(stderr, "Fork Failed");
		return;
	} else if (pid == 0) {
		/* child process */
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(0);
	} else {
		/* parent process */
		/* parent will wait for the child to complete */
		wait(NULL);
		/*printf("Child Complete\n");*/
	}
}

/* Execute looks for the command specified by filename.
 * The order it looks for the command is:
 * 	1. Among built-in commands,
 * 	2. In the PATH.
 *
 * Arguments are passed as an array where the first element
 * is the name of the command we want to run and the following
 * elements are arguments to that command. */
void Execute(char *argv[]) {
    int i;
    
    //printf("%d", checkAlias(argv[0]));
    
    /* Let's make sure there's actually something in the array! */
    if (!**argv) {
        fprintf(stderr,"No arguments given to Execute()");
        return;
    }
    
    /*Testing purposes*/
    /*if(EQ(argv[0], "alias"))
     {
     internal_command(argv);
     return;
     }*/
    
    /*Checks if the curent command is an alias - also breaks everything*/
    
    i = checkAlias(argv[0]);
    
    if(i >= 0)
    {
        /*printf("The command is an alias, I am now Executing the alias.");
         printf("The Alias name is: %s\n", alias[i].name);*/
        Execute(alias[i].command);
        return;	
    }
    
    if (internal_command(argv) < 0) {
        external_command(argv);
    }
}

int main() {
	char *input;
	char *argv[SZ_ARGV];

	printf("test\n");	
	pathValue = getenv("PATH");
	chdir(getenv("HOME")); /*Changes current working directory to HOME */
	load_history();
	init_alias();
	while (1) {
		input = get_input();
		if (input != NULL){
			tokenise(input, argv);
			Execute(argv);
		}
	}
	return 0;
}
