// Problem 3 Client
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
#include <sys/time.h>

int main(int argc, char* argv[]) {
    int client_fd, reader;
    struct sockaddr_in server_address, client_address;
    int server_address_len = sizeof(server_address), client_address_len = sizeof(client_address);
    char buffer[1500], ack[1];
    int status;
    int str_len;
    pid_t k;
    int drop_when = atoi(argv[2]);
    int file_size, dup_size;
    struct timeval start_time, end_time;

    // Initialization, avoid undefined behavior
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Create Socket
    client_address.sin_family = AF_INET;
    client_address.sin_port = 0;

    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, argv[1], &client_address.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Establish Half Connection
    if (bind(client_fd, (struct sockaddr *)&client_address, sizeof(client_address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Retrieve Port Number
    if (getsockname(client_fd, (struct sockaddr *)&client_address, &client_address_len) == -1) {
        perror("getsockname() failed");
        return -1;
    }

    printf("Local IP address is: %s\n", inet_ntoa(client_address.sin_addr));
    printf("Local port is: %d\n", (int) ntohs(client_address.sin_port));


    // Ready to process request
    int n;
    int cnt = 0;

    file_size = 0;
    dup_size = 0;
    ack[0] = 10; // initialize with a number which is not a sequence number

    int flag = 0;
    while (buffer[0] != 2) {
        cnt++;

        n = recvfrom(client_fd, (char *)buffer, 1500,
                     MSG_WAITALL, (struct sockaddr *) &server_address,
                     &server_address_len);
        buffer[n] = '\0';
        if (flag == 0) {
            gettimeofday(&start_time, 0);
            flag = 1;
        }
        if (ack[0] != buffer[0]) file_size += (n - 1); // exclude the header
        else dup_size += (n - 1);

        ack[0] = buffer[0];
        if (drop_when == -1 || cnt % drop_when != 0) {
            if (sendto(client_fd, (const char *)ack, 1,
                       MSG_CONFIRM, (const struct sockaddr *) &server_address,
                       server_address_len) == -1) printf("Fail to send ACK!\n");
        }
    }

    gettimeofday(&end_time, 0);
    long long comp_time = ((end_time.tv_sec-start_time.tv_sec) * 1000000LL + end_time.tv_usec-start_time.tv_usec) / 1000LL;

    printf("========= Closing socket... =========\n");
    close(client_fd);

    // Print transmission statics
    printf("========= Printing statics... =========\n");
    printf("Completion time: %lld milliseconds\n", comp_time);
    printf("Total bytes received from server: %d bytes\n", file_size);
    printf("Duplicate bytes received from server: %d bytes\n", dup_size);
    printf("Transmission speed: %.2f bps\n", 1.00 * file_size / (comp_time) * 1000);
    fflush(stdout);

    return 0;
}
