// Problem 2 Client
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/time.h>

int main(int argc, char const *argv[]) {
    int server_fd = 0, reader;
    struct sockaddr_in server_address;
    int server_address_len = sizeof(server_address);
    char buffer[100];

    memset(&server_address, 0, server_address_len);

    // Parse the input from cmd
    if (argc == 1)
        printf("No Extra Command Line Argument Passed Other Than Program Name\n");
    if (argc >= 2) {
        for (int i = 3; i < argc; i++) {
            strcat(buffer, argv[i]);
            if (i != argc - 1) strcat(buffer, " ");
        }
    }

    // Filling server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }



    // Process response
    // Allocate memory dynamically and read in while loop (read and write in pipeline is concurrent)
    // Detect the ending by the return value of read function
    int cnt = 0;
    const int unit_size = 100;
    char *response;
    fd_set set;
    struct timeval timeout;
    int read_ready, flag;


    // Three attempts
    for (int i = 0; i < 3; i++) {
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        if (connect(server_fd, (struct sockaddr *)&server_address, server_address_len) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }

        // Send request
        write(server_fd, buffer, strlen(buffer) + 1);

        /* Initialize the file descriptor set. */
        FD_ZERO (&set);
        FD_SET (server_fd, &set);

        /* Initialize the timeout data structure. */
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        /* select returns 0 if timeout, 1 if input available, -1 if error. */
        read_ready = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
        flag = read(server_fd, buffer, unit_size - 1);
        if (read_ready == 1 && flag != 0) break;
    }

    if (flag == 0) {
        printf("%s\n", "No response from the server...exit...");
        fflush(stdout);
        exit(1);
    }

//    flag = read(server_fd, buffer, unit_size - 1);
    int response_len = 100;

    response = (char*)malloc(unit_size * sizeof(char));

    while (flag > 0) {
        buffer[flag] = '\0';
        strcat(response, buffer);
        response = (char*)realloc(response, response_len + unit_size);
        response_len += unit_size;
        cnt += flag;
        flag = read(server_fd, buffer, unit_size - 1);
    }

    printf("%s", response);
    fflush(stdout);
    close(server_fd);
    return 0;
}