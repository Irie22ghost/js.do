#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void printPrompt();
char* parser();
char** tokenizer(char* commands);
int executeCommand(char** args, int in, int out);

typedef struct 
{		
	char *key, *value;		//key = alias user wants to use; value = command shortcut will execute
} Shortcut;

Shortcut *shortcuts[75];
int freePosition = 0;

int exitShell();
int changeDirectory(char* dir);
int printWorkingDirectory();
int set(char *shortcut, char *meaning);
int list();
int pipes();

int main(int argc, char **argv)		//argc=# of arguments; argv=cmd line arguments
{
	char *cmd;
	char **args;
	int status = 1;
	int in, out = 0;

	while (status)
	{
		printPrompt();
		cmd = parser();
		if (strpbrk (cmd, ">"))
			out = 1;
		else if (strpbrk (cmd, "<"))
			in = 1;
		
		args = tokenizer(cmd);
		status = executeCommand(args, in, out);

		free(cmd);
		free(args);
		in=out=0;
	}
	return 0;
}

void printPrompt()
{
	char cwd[256];

	getcwd(cwd, sizeof(cwd));
	printf("%s$ ", cwd);
	fflush(stdout);
}

char* parser()
{
	int buffsize = 1024;
	char *line = malloc(sizeof(char) * buffsize);
	fgets(line, buffsize, stdin);
	return line;
}

char** tokenizer(char* commands)
{
	int buffsize = 128, tokencount = 0;
	char **tokens = malloc (sizeof(char*) * buffsize);
	char *token, *delim;

	if(strpbrk (commands, "<>"))
		delim = "<>\n";
	else
		delim = " \n";
	
	token = strtok(commands, delim);
	while(token!=NULL)
	{
		tokens[tokencount] = token;
		tokencount += 1;
		token = strtok(NULL, delim);
	}

	tokens[tokencount] = NULL;
	return tokens;
}

int set(char *shortcut, char *meaning)
{
	Shortcut s = {shortcut, meaning};	//create new alias
	*shortcuts[freePosition] = s;		//add new alias to list
	freePosition++;			//adjust next available opening in list
	return 1;
}

int list()
{
	printf("Shortcut \t\tCommand\n");
	
	for (int i=0; i<freePosition; i++){
		printf("%s \t\t%s\n", shortcuts[i]->key, shortcuts[i]->value);
	}
	return 1;
}

int pipes()
{
}

int executeCommand(char** args, int in, int out)
{	
	for (int i=0;i<freePosition; i++){
		if (strcmp(args[0], shortcuts[i]->key) == 0){
			char **args = tokenizer(shortcuts[i]->value);
			return executeCommand(args, in, out);
		}
	}

	if (strcmp(args[0], "exit") == 0)
	{
		return exitShell();
	} 
	else if (strcmp(args[0], "cd") == 0)
	{
		return changeDirectory(args[1]);
	} 
	else if (strcmp(args[0], "pwd") == 0)
	{
		return printWorkingDirectory();
	}
	else if (strcmp(args[0], "set") == 0){
		if (sizeof(args) < 2)
		{
			printf("Usage: set shortcut command");
			return 1;
		}
		else
		{
			return set(args[1], args[2]);
		}
	}
	else if (strcmp(args[0], "list") == 0)
	{
		return list();
	}
	else {				

		pid_t parent, child;
		int child_status;

		child = fork();
		if(child < 0)
		{
			printf("Fork failed\n");
			exit(-1);
		} 
		else if (child == 0) {
			if (in + out){
				char **cmd_t = tokenizer(args[0]);
				if (in) {
					FILE* infile = fopen(args[1]+1, "r");
					dup2(fileno(infile), 0);
					fclose(infile);
				} 
				else if (out) {
					FILE* outfile = fopen(args[1]+1, "w");
					dup2(fileno(outfile), 1);
					fclose(outfile);
				}
				execvp(cmd_t[0], cmd_t);
			} 
			else {
				execvp(args[0], args);
			}

			printf("Error %d (%s)\n", errno, strerror(errno));
			exit(1);
		} 
		else {
			parent = wait(&child_status);
			fflush(stdout);
			return 1;
		}
	}
}

int exitShell()
{
	printf("Exiting\n");
	return 0;
}

int changeDirectory(char* dir)
{
	int cd_status;
	cd_status = chdir(dir);

	if (cd_status == -1)
	{
		printf("Error %d (%s)\n", errno, strerror(errno));
	}
	return 1;
}

int printWorkingDirectory()
{
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);
	fflush(stdout);

	return 1;
}




























