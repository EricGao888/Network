// Problem 1 Client
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char* argv[]) {
    int client_fd = 0, reader;
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

    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Filling server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr)<=0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    sendto(client_fd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &server_address,
           server_address_len);

    close(client_fd);

    return 0;
}