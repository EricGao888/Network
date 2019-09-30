// Problem 2 Server
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char const *argv[]) {

    srand(time(0));

    int server_fd, client_fd, reader;
    struct sockaddr_in server_address, client_address;
    int opt = 1;
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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr)<=0) {
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

    // Socket ready and listen
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }



//    send(new_socket , hello , strlen(hello) , 0 );


    // Read to process request
    while(1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&client_address_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        reader = read(client_fd, buffer, sizeof(buffer));
        // print message from client
        printf("Message: %s\n", buffer);

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

        // Flip the coin
        int coin = rand() & 1;
        
        if (coin == 1) {
            close(client_fd);
            printf("%s\n", "Client request ignored...");
            continue;
        }

        // Create new child process and perform system call
        k = fork();

        // Child code
        if (k==0) {
            // Close duplicated half connection in child process
            close(server_fd);
            // Redirect stdout to Socket, and close this duplicated full connection descriptor
            dup2(client_fd, 1);
            close(client_fd);

            // If execution failed, terminate child
            if(execvp(arguments[0], arguments) == -1) {
                exit(1);
            }
        }
        // Close original full connection descriptor, if not, client read function will be blocked
        close(client_fd);
        printf("%s\n", "Client request processed...");
        fflush(stdout);
    }
    return 0;
} 