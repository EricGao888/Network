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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void) {
    int server_fd, new_fd, reader;
    struct sockaddr_in server_address, client_address;
    int server_address_len = sizeof(server_address), client_address_len = sizeof(client_address);
    char buffer[100];
    int status;
    int str_len;
    pid_t k;

    // Initialization, avoid undefined behavior
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Create Socket
    server_address.sin_family = AF_INET;
    server_address.sin_port = 0;

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, "128.10.25.201", &server_address.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Establish Half Connection
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Retrieve Port Number
    if (getsockname(server_fd, (struct sockaddr *)&server_address, &server_address_len) == -1) {
        perror("getsockname() failed");
        return -1;
    }

    printf("Local IP address is: %s\n", inet_ntoa(server_address.sin_addr));
    printf("Local port is: %d\n", (int) ntohs(server_address.sin_port));


    // Read to process request
    while(1) {
        int n;
        n = recvfrom(server_fd, (char *)buffer, 100,
                     MSG_WAITALL, ( struct sockaddr *) &client_address,
                     &client_address_len);
        buffer[n] = '\0';
        // print message from client
//        printf("Message: %s\n", buffer);

        // Parse arguments
        char *arguments[strlen(buffer)];
        char *token;
        token = strtok(buffer, " ");
        int idx = 0;
        while (token != NULL) { 
            arguments[idx++] = token;
            token = strtok(NULL, " ");
        }
        arguments[idx] = NULL;

        // Create new child process and perform system call

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
