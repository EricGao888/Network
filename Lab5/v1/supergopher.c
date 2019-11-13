// Problem 1 Server
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
#include "supergopher.h"

#define MAXSOCKIND 10

struct tuple {
    int socket_index;
    unsigned short transit_port;
    unsigned short transit_port2;
    unsigned long client_IP;
    unsigned short client_port;
    unsigned long server_IP;
    unsigned short server_port;
};

int master_fd, fd_cnt, max_fd, activity, n, socket_idx;
struct sockaddr_in mini_address, server_address, client_address, remote_address;
socklen_t mini_address_len, remote_address_len, client_address_len;
struct tuple table[MAXSOCKIND];
int fds[MAXSOCKIND];
fd_set fdset;
char buffer[100];
unsigned short transit_port, transit_port2;

unsigned short create_socket(int *fd, unsigned short port_num);

void clean(char arr[], int size);

int main(int argc, char* argv[]) {

    // Initialization, avoid undefined behavior
    memset(&mini_address, 0, sizeof(mini_address));
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));
    memset(&remote_address, 0, sizeof(remote_address));
    memset(&fds, 0, sizeof(fds));
    FD_ZERO(&fdset);
    fd_cnt = 0;
    clean(buffer, sizeof(buffer));

    // Establish master socket
    create_socket(&master_fd, htons(atoi(argv[1])));
    max_fd = master_fd;

    // Read to process request
    while(1) {
        // Set up file descriptors
        FD_ZERO(&fdset);
        FD_SET(master_fd, &fdset);
        for (int i = 0; i < fd_cnt; i++) {
            FD_SET(fds[i], &fdset);
        }

        // Wait for an activity on one of the sockets, timeout is NULL , so wait indefinitely
        printf("Waiting for activity...\n");
        fflush(stdout);
        activity = select(max_fd + 1 , &fdset , NULL , NULL , NULL);

        printf("Activity detected!\n");
        fflush(stdout);

        if (activity < 0) {
            printf("select error!\n");
        }

        //If something happened on the master socket, then it is a new connection request
        if (FD_ISSET(master_fd, &fdset)) {
            printf("New connection request received!\n");
            fflush(stdout);

            // Store msg
            clean(buffer, sizeof(buffer));
            remote_address_len = sizeof(remote_address);
            if ((n = recvfrom(master_fd, (char *)buffer, sizeof(buffer),
                              MSG_WAITALL, (struct sockaddr *) &remote_address,
                              &remote_address_len)) < 0) {
                printf("recvfrom error!\n");
            }

            // No more tunnels
            if (fd_cnt == MAXSOCKIND) {
                clean(buffer, sizeof(buffer));
                remote_address_len = sizeof(remote_address);
                buffer[0] = 0;
                if (sendto(master_fd, (const char *)buffer, 1,
                           0, (const struct sockaddr *) &remote_address,
                           remote_address_len) < 0) printf("Fail to send message!\n");
                printf("Reject connection due to limit on number of sockets!\n");
                continue;
            }

            // Establish new tunnel
            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;
            for (int i = 0; i <= 3; i++) {
                server_address.sin_addr.s_addr = (((unsigned char)buffer[i] << ((3 - i) * 8)) | server_address.sin_addr.s_addr);
            }
            for (int i = 4; i <= 5; i++) {
                server_address.sin_port = (((unsigned char)buffer[i] << ((5 - i) * 8)) | server_address.sin_port);
            }
            printf("The IP address of target server is: %s\n", inet_ntoa(server_address.sin_addr));
            printf("The port of target server port is: %hu\n", ntohs(server_address.sin_port));
            fflush(stdout);

            // Create tunnel for client and server
            transit_port = create_socket(&fds[fd_cnt], 0);
            transit_port2 = create_socket(&fds[fd_cnt + 1], 0);
//            FD_SET(fds[fd_cnt], &fdset);
//            FD_SET(fds[fd_cnt + 1], &fdset);
            max_fd = max_fd > fds[fd_cnt] ? max_fd : fds[fd_cnt];
            max_fd = max_fd > fds[fd_cnt + 1] ? max_fd : fds[fd_cnt + 1];
            table[fd_cnt].socket_index = fd_cnt; // mapping
            table[fd_cnt + 1].socket_index = fd_cnt + 1; // mapping
            table[fd_cnt].server_IP = server_address.sin_addr.s_addr;
            table[fd_cnt + 1].server_IP = server_address.sin_addr.s_addr;
            table[fd_cnt].server_port = server_address.sin_port;
            table[fd_cnt + 1].server_port = server_address.sin_port;
            table[fd_cnt].transit_port = transit_port;
            table[fd_cnt + 1].transit_port = transit_port;
            table[fd_cnt].transit_port2 = transit_port2;
            table[fd_cnt + 1].transit_port = transit_port2;
            table[fd_cnt].client_IP = remote_address.sin_addr.s_addr;
            table[fd_cnt + 1].client_IP = remote_address.sin_addr.s_addr;
            fd_cnt += 2;
            if (TABLEUPDATE == 1) printf("Forwarding table updated!\n");

            clean(buffer, sizeof(buffer));
            mini_address_len = sizeof(mini_address);
            buffer[0] = 3;
            buffer[1] = (transit_port >> 8) & 255;
            buffer[2] = transit_port & 255;
//            printf("test: %hu\n", ntohs(transit_port));
            if (sendto(master_fd, (const char *)buffer, 3,
                       0, (const struct sockaddr *) &remote_address,
                       remote_address_len) < 0) printf("Fail to send message!\n");
        }
        else {
            for (int idx = 0; idx < MAXSOCKIND; idx++) {
                if (FD_ISSET(fds[idx], &fdset)) {
                    // Store msg
                    clean(buffer, sizeof(buffer));
                    remote_address_len = sizeof(remote_address);
                    if ((n = recvfrom(fds[idx], (char *)buffer, sizeof(buffer),
                                      0, (struct sockaddr *) &remote_address,
                                      &remote_address_len)) < 0) {
                        printf("recvfrom error!\n");
                    }

                    if (idx % 2 == 0) {
                        printf("Client message received!\n");
                        fflush(stdout);
                        if (table[idx].client_IP != remote_address.sin_addr.s_addr) {
                            printf("Client IP does not match and the message is discarded!\n");
                            continue;
                        }
                        table[idx].client_port = remote_address.sin_port;
                        table[idx + 1].client_port = remote_address.sin_port;
                        if (TABLEUPDATE == 1) printf("Forwarding table updated!\n");
                        memset(&server_address, 0, sizeof(server_address));
                        server_address.sin_family = AF_INET;
                        server_address.sin_addr.s_addr = table[idx].server_IP;
                        server_address.sin_port = table[idx].server_port;
                        if (sendto(fds[idx + 1], (const char *)buffer, n,
                                   0, (const struct sockaddr *) &server_address,
                                   sizeof(server_address)) < 0) printf("Fail to send message!\n");
                    }
                    else {
                        printf("Server message received!\n");
                        fflush(stdout);
                        memset(&client_address, 0, sizeof(client_address));
                        client_address.sin_family = AF_INET;
                        client_address.sin_addr.s_addr = table[idx].client_IP;
                        client_address.sin_port = table[idx].client_port;
                        client_address_len = sizeof(client_address);
                        if (sendto(fds[idx - 1], (const char *)buffer, n,
                                   0, (const struct sockaddr *) &client_address,
                                   client_address_len) < 0) printf("Fail to send message!\n");
                    }
                    break;
                }
            }
        }
    }
    return 0;

}

unsigned short create_socket(int *fd, unsigned short port_num) {

    // Create Socket
    struct sockaddr_in address;
    int address_len;
    memset(&address, 0, sizeof(address));
    address_len = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port_num;


    if ((*fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket with address
    if (bind(*fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    getsockname(*fd, (struct sockaddr *)&address, &address_len);

//    if (*fd != master_fd) {
//        printf("New port is: %hu\n", ntohs(address.sin_port));
//    }
//    else {
//        printf("Local IP address is: %s\n", inet_ntoa(address.sin_addr));
//        printf("Local port is: %hu\n", ntohs(address.sin_port));
//    }
    return address.sin_port;
}

void clean(char arr[], int size) {
    memset(arr, 0, sizeof(unsigned char) * size);
}