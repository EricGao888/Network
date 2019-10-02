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

int server_fd = 0, reader;
struct sockaddr_in client_address;
int client_address_len = sizeof(client_address);
char buffer[1500], ack[1];
int file_size, file_left;
int block_size;
int time_out;
struct itimerval it_val;
int header = 0;
int flag = 3;

void send_package(void);
void set_timer(void);
void cancel_timer(void);

int main(int argc, char* argv[]) {

    file_size = atoi(argv[1]);
    block_size = atoi(argv[2]);
    time_out = atoi(argv[3]);

    // Set timer
    if (signal(SIGALRM, (void (*)(int)) send_package) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }

    memset(&client_address, 0, client_address_len);
    memset(buffer, '3', sizeof(buffer));

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

    printf("Start to send file...\n");
    file_left = file_size;
    ack[0] = 10;
    for (file_left = file_size; file_left > 0; file_left -= block_size) {
        send_package();
        set_timer();
        while (ack[0] != header) {
            recvfrom(server_fd, (char *)ack, 1,
                     MSG_WAITALL, (struct sockaddr *) &client_address,
                     &client_address_len);
        }
        cancel_timer();
        header ^= 1;
    }

    close(server_fd);
    printf("File successfully sent!\n");

    return 0;
}

void send_package(void) {
    if (flag == 0) exit(1);
//    printf("Sending package...\n");
    fflush(stdout);

    int info_size; // info = header + actual info

    if (file_left > block_size) info_size = block_size + 1;
    else {
        info_size = file_left + 1;
        header = 2;
        flag--;
    }
//    printf("File bytes left: %d\n", file_left);

    buffer[0] = header;

    if (sendto(server_fd, (const char *)buffer, info_size,
               MSG_CONFIRM, (const struct sockaddr *) &client_address,
               client_address_len) == -1) printf("Fail to send information...\n");
    fflush(stdout);
}

void set_timer(void) {
    it_val.it_value.tv_sec = time_out / 1000000;
    it_val.it_value.tv_usec = time_out % 1000000;
    it_val.it_interval = it_val.it_value;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }
}

void cancel_timer(void) {
    if (setitimer(ITIMER_REAL, NULL, NULL) == -1) {
        perror("error cancelling setitimer()");
        exit(1);
    }
}