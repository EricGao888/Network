// Problem 1 Server
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

#define MAXBUFSZM 1000000

int main(int argc, char const *argv[]) {

    srand(time(0));

    int server_fd, client_fd, fd, cnt;
    struct sockaddr_in server_address, client_address;
    int server_address_len = sizeof(server_address), client_address_len = sizeof(client_address);
    int status, str_len, block_size;
    char buffer[MAXBUFSZM];
    char flag[1];
    int file_size;

    pid_t k;

    file_size = 0;
    block_size = atoi(argv[1]) < MAXBUFSZM ? atoi(argv[1]) : MAXBUFSZM;

    // Initialization, avoid undefined behavior
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Create Socket
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
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



//    printf("Local IP address is: %s\n", inet_ntoa(server_address.sin_addr));
//    printf("Local port is: %d\n", (int) ntohs(server_address.sin_port));

    // Socket ready and listen
    if (listen(server_fd, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server starts...\n");

    // Read to process request
    while(1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&client_address_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create new child process and perform system call
        k = fork();

        // Child code
        if (k==0) {
            // Close duplicated half connection in child process
            close(server_fd);

            // Read filename from client
            cnt = read(client_fd, buffer, sizeof(buffer));
            printf("Filename: %s\n", buffer);

            // Read from file
            fd = open((char*)buffer, O_RDONLY);
            if (fd == -1) {
                flag[0] = '0';
                write(client_fd, flag, 1);
            }
            else {
                cnt = read(fd, buffer, block_size);
                if (cnt == 0) {
                    flag[0] = '1';
                    write(client_fd, flag, 1);
                }
                else {
                    flag[0] = '2';
                    write(client_fd, flag, 1);
                }
                while (cnt > 0) {
                    file_size += cnt;
                    write(client_fd, buffer, cnt);
                    cnt = read(fd, buffer, block_size);
                }
                if (file_size != 0) printf("File size: %d bytes\n", file_size);
            }
            close(fd);
            close(client_fd);
            exit(0);
        }
        // Close original full connection descriptor, if not, client read function will be blocked
        close(client_fd);
        printf("%s\n", "Client request processed...");
        fflush(stdout);
    }
    return 0;
} 