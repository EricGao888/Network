// Problem 3 Server
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
#include <sys/time.h>
#include <signal.h>

void send_package(void);

int main(int argc, char* argv[]) {
    int server_fd = 0, reader;
    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);
    char buffer[1500], ack[1];
    int file_size = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    int time_out = atoi(argv[3]);
    struct itimerval it_val;

    // Set timer
    if (signal(SIGALRM, (void (*)(int)) send_package) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }
    it_val.it_value.tv_sec = time_out / 1000000;
    it_val.it_value.tv_usec = time_out % 1000000;
    it_val.it_interval = it_val.it_value;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }

    memset(&client_address, 0, client_address_len);
    memset(buffer, 'a', sizeof(buffer));

    // Parse the input from cmd
    if (argc == 1)
        printf("No Extra Command Line Argument Passed Other Than Program Name\n");
//    if (argc >= 2) {
//        for (int i = 3; i < argc; i++) {
//            strcat(buffer, argv[i]);
//            if (i != argc - 1) strcat(buffer, " ");
//        }
//    }

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Filling server information
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(atoi(argv[5]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[4], &client_address.sin_addr)<=0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    int header = 0;
    for (int file_left = file_size; file_left > 0; file_left -= block_size) {
        int info_size; // info = header + actual info

        if (file_left > block_size) info_size = block_size + 1;
        else {
            info_size = file_left + 1;
            header = 2;
        }

        buffer[0] = header;

        if (sendto(server_fd, (const char *)buffer, info_size,
               MSG_CONFIRM, (const struct sockaddr *) &client_address,
               client_address_len) == -1) printf("Fail to send information...\n");

        recvfrom(server_fd, (char *)ack, 1,
                 MSG_WAITALL, (struct sockaddr *) &client_address,
                 &client_address_len);

        if (ack[0] != header) printf("Package Lost!\n");

        header ^= 1;
    }

    int n;
    n = recvfrom(server_fd, (char *)buffer, 100,
                 MSG_WAITALL, (struct sockaddr *) &client_address,
                 &client_address_len);
    buffer[n] = '\0';
    printf("Message from client: %s\n", buffer);

    close(server_fd);

    return 0;
}

void send_package(void) {
    printf("Resend package...\n");
    fflush(stdout);
}