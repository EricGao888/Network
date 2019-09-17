// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

int main(void) {
    pid_t k;
    char buf[100];
    int status;
    int len;

    /**
    * Create FIFO
    */
    int fd; // the position where to read/write
    char * fifo = "server_queue";
    mkfifo(fifo, 0666);

    while(1) {

    	// print prompt
      	fprintf(stdout,"[%d]$ ",getpid());

        // Open in read only and read 
        fd = open(fifo,O_RDONLY); 
        read(fd, buf, sizeof(buf)); 
        len = strlen(buf); 
        if(len == 1)    
            continue;
        buf[len-1] = '\0';
        // Print the read string and close
        // printf("Message: %s\n", buf);  
        close(fd); 

        /**
        * parsing arguments
        */
        char *arguments[strlen(buf)];
        char *token;
        token = strtok(buf, " ");
        int idx = 0;
        while (token != NULL) { 
            arguments[idx++] = token;
            token = strtok(NULL, " ");
        }
        arguments[idx] = NULL;

        /**
        * Create new child process and perform system call
        */
    	k = fork();
        // child code
    	if (k==0) {
            // if execution failed, terminate child
            if(execvp(arguments[0], arguments) == -1) {
                exit(1);
            }	
    	}
        // parent code 
    	// else {
            // waitpid(k, &status, 0);
    	// }
    }
    return 0;
}
