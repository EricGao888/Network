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
#include "overlaygopher.h"

#define MAXSOCKIND 10

struct tuple {
    int socket_index;
    unsigned long pre_IP;
    unsigned short pre_port;
    unsigned long post_IP;
    unsigned short post_port;
};

int master_fd, fd_cnt, max_fd, activity, n, socket_idx;
struct sockaddr_in mini_address, post_address, pre_address, remote_address;
socklen_t mini_address_len, remote_address_len, pre_address_len, post_address_len;
struct tuple table[MAXSOCKIND];
int fds[MAXSOCKIND];
fd_set fdset;
char buffer[100];
unsigned long ip;
unsigned short port, pre_port, post_port, transit_port, transit_port2;
char *fetcher;
char k;

unsigned short create_socket(int *fd, unsigned short port_num);

void clean(char arr[], int size);

int main(int argc, char* argv[]) {

    // Initialization, avoid undefined behavior
    memset(&mini_address, 0, sizeof(mini_address));
    memset(&post_address, 0, sizeof(post_address));
    memset(&pre_address, 0, sizeof(pre_address));
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

            // Acquire address of the post machine
            k = buffer[0];
            memset(&post_address, 0, sizeof(post_address));
            fetcher = buffer + 10;
            post_address.sin_family = AF_INET;
            for (int i = 0; i <= 3; i++) {
                post_address.sin_addr.s_addr = (((unsigned char)*fetcher << ((3 - i) * 8)) | post_address.sin_addr.s_addr);
                fetcher++;
            }
            fetcher++;
            for (int i = 0; i <= 1; i++) {
                post_address.sin_port = (((unsigned char)*fetcher << ((1 - i) * 8)) | post_address.sin_port);
                fetcher++;
            }
            printf("The IP address of post machine is: %s\n", inet_ntoa(post_address.sin_addr));
            printf("The port of post machine port is: %hu\n", ntohs(post_address.sin_port));
            fflush(stdout);

            // Update forwarding table
            transit_port = create_socket(&fds[fd_cnt], 0); // Used to receive file from previous machine
            transit_port2 = create_socket(&fds[fd_cnt + 1], 0); // Used to send file to post machine
            max_fd = max_fd > fds[fd_cnt] ? max_fd : fds[fd_cnt];
            max_fd = max_fd > fds[fd_cnt + 1] ? max_fd : fds[fd_cnt + 1];
            table[fd_cnt].socket_index = fd_cnt; // mapping
            table[fd_cnt + 1].socket_index = fd_cnt + 1; // mapping
            table[fd_cnt].post_IP = post_address.sin_addr.s_addr;
            table[fd_cnt + 1].post_IP = post_address.sin_addr.s_addr;;
            table[fd_cnt].pre_IP = remote_address.sin_addr.s_addr;
            table[fd_cnt + 1].pre_IP = remote_address.sin_addr.s_addr;
            fd_cnt += 2;
            if (TABLEUPDATE == 1) printf("Forwarding table updated!\n");

            // Send connection request to the post machine
            if (k != 1) {
                fflush(stdout);
                buffer[8] = k - 1;
                if (sendto(fds[fd_cnt - 1], (const char *)(buffer + 8), n - 8,
                           0, (const struct sockaddr *) &post_address,
                           sizeof(post_address)) < 0) printf("Fail to send message!\n");
                clean(buffer, sizeof(buffer));
                post_address_len = sizeof(post_address_len);

                // Receive ACK from the post machine
                if ((n = recvfrom(fds[fd_cnt - 1], (char *)buffer, sizeof(buffer),
                                  MSG_WAITALL, (struct sockaddr *) &post_address,
                                  &post_address_len)) < 0) {
                    printf("recvfrom error!\n");
                }

                fflush(stdout);
                if (buffer[0] != 3) {
                    printf("Connection rejected!\n");
                    clean(buffer, sizeof(buffer));
                    remote_address_len = sizeof(remote_address);
                    buffer[0] = 0;
                    if (sendto(master_fd, (const char *)buffer, 1,
                               0, (const struct sockaddr *) &remote_address,
                               remote_address_len) < 0) printf("Fail to send message!\n");
                    continue;
                }
                else {
                    post_port = 0;
                    post_port = ((unsigned char)buffer[2] | post_port);
                    post_port = (((unsigned char)buffer[1] << 8) | post_port);
                    table[fd_cnt - 2].post_port = post_port;
                    table[fd_cnt - 1].post_port = post_port;
                    printf("Assigned transit-port is: %hu\n", ntohs(post_port));
                }
            }
            else {
                table[fd_cnt - 2].post_port = post_address.sin_port;
                table[fd_cnt - 1].post_port = post_address.sin_port;
            }

            // Assign port to the previous machine
            clean(buffer, sizeof(buffer));
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
                        printf("Pre message received!\n");
                        fflush(stdout);
                        if (table[idx].pre_IP != remote_address.sin_addr.s_addr) {
                            printf("Client IP does not match and the message is discarded!\n");
                            continue;
                        }
                        table[idx].pre_port = remote_address.sin_port;
                        table[idx + 1].pre_port = remote_address.sin_port;
                        if (TABLEUPDATE == 1) printf("Forwarding table updated!\n");
                        memset(&post_address, 0, sizeof(post_address));
                        post_address.sin_family = AF_INET;
                        post_address.sin_addr.s_addr = table[idx].post_IP;
                        post_address.sin_port = table[idx].post_port;
                        if (sendto(fds[idx + 1], (const char *)buffer, n,
                                   0, (const struct sockaddr *) &post_address,
                                   sizeof(post_address)) < 0) printf("Fail to send message!\n");
                    }
                    else {
                        printf("Post message received!\n");
                        fflush(stdout);
                        memset(&pre_address, 0, sizeof(pre_address));
                        pre_address.sin_family = AF_INET;
                        pre_address.sin_addr.s_addr = table[idx].pre_IP;
                        pre_address.sin_port = table[idx].pre_port;
                        pre_address_len = sizeof(pre_address);
                        if (sendto(fds[idx - 1], (const char *)buffer, n,
                                   0, (const struct sockaddr *) &pre_address,
                                   pre_address_len) < 0) printf("Fail to send message!\n");
                    }
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