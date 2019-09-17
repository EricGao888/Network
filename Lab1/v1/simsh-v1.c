// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(void) {
    pid_t k;
    char buf[100];
    int status;
    int len;

    while(1) {

	// print prompt
  	fprintf(stdout,"[%d]$ ",getpid());

	// read command from stdin
	fgets(buf, 100, stdin);
	len = strlen(buf);
	if(len == 1) 				// only return key pressed
	  continue;
	buf[len-1] = '\0';

    // printf("input is: %s\n", buf);
    char *arguments[strlen(buf)];
    // char *cmd;
    char *token;
    token = strtok(buf, " ");
    // cmd = token;
    int idx = 0;

    while (token != NULL) { 
        arguments[idx++] = token;
        token = strtok(NULL, " ");
        // printf("%s\n", token);
    }

    arguments[idx] = NULL;
    // printf("%d\n", idx);
    	k = fork();
        // child code
    	if (k==0) {
            // if execution failed, terminate child
            if(execvp(arguments[0], arguments) == -1) {
                exit(1);
            }	
    	}
        // parent code 
    	else {
            waitpid(k, &status, 0);
    	}
    }
}
