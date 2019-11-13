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

int fd = 0, n;
struct sockaddr_in super_address, server_address;
socklen_t super_address_len;
char buffer[100];
unsigned short port_num;

void clean(char arr[], int size);

int main(int argc, char* argv[]) {

    memset(&server_address, 0, sizeof(server_address));
    memset(&super_address, 0, sizeof(super_address));
    super_address_len = sizeof(super_address);
    clean(buffer, sizeof(buffer));

    // Filling server information
    super_address.sin_family = AF_INET;
    super_address.sin_port = htons(atoi(argv[2]));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[4]));

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &super_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }
    if(inet_pton(AF_INET, argv[3], &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    // Prepare msg
    for (int i = 0; i <= 3; i++) {
        buffer[i] = (server_address.sin_addr.s_addr >> 8 * (3 - i)) & 255;
    }

    for (int i = 4; i <= 5; i++) {
        buffer[i] = (server_address.sin_port >> 8 * (5 - i)) & 255;
    }

    if (sendto(fd, (const char *)buffer, 6,
           0, (const struct sockaddr *) &super_address,
           super_address_len) < 0) printf("Fail to send message!\n");
    super_address_len = sizeof(super_address);

    clean(buffer, sizeof(buffer));
    if ((n = recvfrom(fd, (char *)buffer, sizeof(buffer),
                      MSG_WAITALL, (struct sockaddr *) &super_address,
                      &super_address_len)) < 0) {
        printf("recvfrom error!\n");
    }
    if (buffer[0] != 3) {
        printf("Connection rejected!\n");
    }
    else {
        port_num = 0;
        port_num = ((unsigned char)buffer[2] | port_num);
        port_num = (((unsigned char)buffer[1] << 8) | port_num);
        printf("Assigned transit-port is: %hu\n", ntohs(port_num));
    }

    close(fd);

    return 0;
}

void clean(char arr[], int size) {
    memset(arr, 0, sizeof(unsigned char) * size);
}