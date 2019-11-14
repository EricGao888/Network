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

int fd = 0, n, k;
struct sockaddr_in router_address, dst_address, tmp_address;
socklen_t router_address_len;
char buffer[100];
unsigned long ip;
unsigned short port;

void clean(char arr[], int size);

int main(int argc, char* argv[]) {

    memset(&dst_address, 0, sizeof(dst_address));
    memset(&router_address, 0, sizeof(router_address));
    memset(&tmp_address, 0, sizeof(tmp_address));
    router_address_len = sizeof(router_address);
    clean(buffer, sizeof(buffer));
    k = (argc - 3) / 2;
    buffer[0] = k;

    // Filling dst information
    router_address.sin_family = AF_INET;
    if(inet_pton(AF_INET, argv[1], &router_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }
    router_address.sin_port = htons(atoi(argv[2]));

    dst_address.sin_family = AF_INET;
    if(inet_pton(AF_INET, argv[argc - 2], &dst_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }
    dst_address.sin_port = htons(atoi(argv[argc - 1]));

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Prepare msg
    int idx = 1;
    for (int i = 1; i < argc; i++) {
        buffer[idx++] = '#';
        if (i % 2 == 1) {
            tmp_address.sin_family = AF_INET;
            if(inet_pton(AF_INET, argv[i], &tmp_address.sin_addr) <= 0) {
                printf("Invalid address/ Address not supported \n");
                return -1;
            }
            ip = tmp_address.sin_addr.s_addr;
            for (int offset = 0; offset <= 3; offset++) {
                buffer[idx + offset] = (ip >> 8 * (3 - offset)) & 255;
            }
            idx += 4;
        }
        else {
            port = htons(atoi(argv[i]));
            for (int offset = 0; offset <= 1; offset++) {
                buffer[idx + offset] = (port >> 8 * (1 - offset)) & 255;
            }
            idx += 2;
        }
    }

    if (sendto(fd, (const char *)buffer, idx,
           0, (const struct sockaddr *) &router_address,
           router_address_len) < 0) printf("Fail to send message!\n");
    router_address_len = sizeof(router_address);

    clean(buffer, sizeof(buffer));
    if ((n = recvfrom(fd, (char *)buffer, sizeof(buffer),
                      MSG_WAITALL, (struct sockaddr *) &router_address,
                      &router_address_len)) < 0) {
        printf("recvfrom error!\n");
    }
    if (buffer[0] != 3) {
        printf("Connection rejected!\n");
    }
    else {
        port = 0;
        port = ((unsigned char)buffer[2] | port);
        port = (((unsigned char)buffer[1] << 8) | port);
        printf("Assigned transit-port is: %hu\n", ntohs(port));
    }

    close(fd);

    return 0;
}

void clean(char arr[], int size) {
    memset(arr, 0, sizeof(unsigned char) * size);
}