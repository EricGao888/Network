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
    char request[100]; // information in server queue
    // char response[100]; // information in client queue
    int status;
    int requestLen;
    // int responseLen;

    /**
    * Create FIFO
    */
    // the position where to read/write
    int fd_ser; // Read from server queue  
    int fd_cli; // Write into cli queue
    char * fifo_ser = "server_queue"; 
    char * fifo_cli = "client_queue"; 
    mkfifo(fifo_ser, 0666);

    while (1) {

    	// print prompt
      	fprintf(stdout,"[%d]$ ",getpid());
        fflush(stdout);

        // Open in read only and read 
        fd_ser = open(fifo_ser,O_RDONLY); 
        read(fd_ser, request, sizeof(request)); 
        requestLen = strlen(request); 
        if(requestLen == 1)    
            continue;
        request[requestLen - 1] = '\0';
        // Print the read string and close
        // printf("Message: %s\n", buf);  
        close(fd_ser); 

        /**
        * parsing arguments
        */
        char *arguments[strlen(request)];
        char *token;
        token = strtok(request, " ");
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

            // Open FIFO for write only 
            fd_cli = open(fifo_cli, O_WRONLY); 
            // Write the input string on FIFO 
            // and close it 
            dup2(fd_cli, 1);
            close(fd_cli); // descriptor is closed but file can still be operated by pointer
            // if execution failed, terminate child
            if(execvp(arguments[0], arguments) == -1) {
                exit(1);
            }	
            // write(fd_cli, "\0", 1); 
            
    	}
        // parent code 
    	else {
            // waitpid(k, &status, 0);
    	}
    }
    return 0;
}
