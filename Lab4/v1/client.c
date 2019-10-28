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

int key_idx = 0;

unsigned char mydecode(unsigned char x, unsigned int prikey);

int main(int argc, char* argv[]) {
    int client_fd = 0, reader;
    struct sockaddr_in server_address;
    int server_address_len = sizeof(server_address);
    unsigned char buffer[100];
    unsigned int prikey;
    char *ptr;

    memset(&server_address, 0, server_address_len);

    if (argc == 1)
        printf("No Extra Command Line Argument Passed Other Than Program Name\n");

    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Filling server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    // Get private key from cmd line
    prikey = strtoul(argv[3], &ptr, 10);
//    printf("Key: %u\n", prikey);

    for (int i = 3; i >= 0; i--) {
        buffer[i] = (server_address.sin_addr.s_addr >> (8 * (3 - i))) & 255; // use the ip of server as request header to perform authentication
    }

    buffer[4] = '\0';

    // Parse the input from cmd
    int cur_len = 0;
    if (argc >= 2) {
        for (int i = 4; i < argc; i++) {
            if (cur_len + strlen(argv[i]) > 30) {
                printf("Illegal request dropped!\n");
                exit(1);
            }
            strcat(buffer, argv[i]);
            cur_len += strlen(argv[i]);
            if (i != argc - 1) strcat(buffer, " ");
        }
    }

    // Create certificate
    for (; key_idx < strlen(buffer); key_idx++) {
        buffer[key_idx] = mydecode(buffer[key_idx], prikey);
    }

    sendto(client_fd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &server_address,
           server_address_len);

    close(client_fd);

    return 0;
}

unsigned char mydecode(unsigned char x, unsigned int prikey) {
    return x ^ (prikey >> ((key_idx++ % 4) * 8) ^ 255);
}