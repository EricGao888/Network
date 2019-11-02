// Problem 2 Server
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
#include "myftpd.h"

#define RTT_weight 0.7


int server_fd = 0, reader;
struct sockaddr_in client_address;
int client_address_len = sizeof(client_address);
char buffer[1500], ack[1];
int file_size, file_left;
int block_size;
double curRTT;
struct itimerval it_val;
int header = 0;
int flag = 3;
int first = 1;
struct timeval time_map[131];
struct timeval time;

void send_package(void);
void resend_package(void);
void set_timer(void);
void cancel_timer(void);
double calculate_time(struct timeval start_time, struct timeval end_time);

int main(int argc, char* argv[]) {

    file_size = atoi(argv[1]);
    block_size = atoi(argv[2]);
    curRTT = atoi(argv[3]);

    // Set timer
    if (signal(SIGALRM, (void (*)(int)) resend_package) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }

    memset(&client_address, 0, client_address_len);
    memset(buffer, '3', sizeof(buffer));
    memset(time_map, 0, sizeof(time_map));

    // Parse the input from cmd
    if (argc == 1)
        printf("No Extra Command Line Argument Passed Other Than Program Name\n");

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
    for (file_left = file_size; file_left > 0; file_left -= block_size) {
        // Send a new pkg
        send_package(); // header changed
        recvfrom(server_fd, (char *)ack, 1,
                 MSG_WAITALL, (struct sockaddr *) &client_address,
                 &client_address_len);
        gettimeofday(&time, 0);

        // Receive the expected ACK
        if (header == ack[0]) {
            cancel_timer();
            double newRTT = calculate_time(time_map[ack[0] + 3], time);
            if (RTTPRINT == 1) {
                printf("newRTT: %.2f\n", newRTT);
                printf("curRTT(old): %.2f\n", curRTT);
            }
            curRTT = RTT_weight * curRTT + (1 - RTT_weight) * newRTT;
            if (RTTPRINT == 1) printf("curRTT(new): %.2f\n", curRTT);
            continue;
        }

        if (header >= 0) {
            // Receive delayed ACK
            if (ack[0] % 2 == header % 2 && ack[0] != header) {
                cancel_timer();
                // calculate new RTT
                double newRTT = calculate_time(time_map[ack[0] + 3], time);
                if (RTTPRINT == 1) {
                    printf("newRTT: %.2f\n", newRTT);
                    printf("curRTT(old): %.2f\n", curRTT);
                }
                curRTT = RTT_weight * curRTT + (1 - RTT_weight) * newRTT;
                if (RTTPRINT == 1) printf("curRTT(new): %.2f\n", curRTT);
            }
            
            // Receive legend ACK
            else if (ack[0] % 2 != header % 2) {
                // Clear legend ACK
                while ((ack[0] % 2) != (header % 2)) {
                    recvfrom(server_fd, (char *)ack, 1,
                             MSG_WAITALL, (struct sockaddr *) &client_address,
                             &client_address_len);
                    gettimeofday(&time, 0);
                    // Get expected ACK and cancel timer
                    if (ack[0] % 2 == (header % 2)) cancel_timer();
                }
                // Compute new RTT
                double newRTT = calculate_time(time_map[ack[0] + 3], time);
                if (RTTPRINT == 1) {
                    printf("newRTT: %.2f\n", newRTT);
                    printf("curRTT(old): %.2f\n", curRTT);
                }
                curRTT = RTT_weight * curRTT + (1 - RTT_weight) * newRTT;
                if (RTTPRINT == 1) printf("curRTT(new): %.2f\n", curRTT);
            }
        }
        else {
            // Receive legend ACK
            while (ack[0] >= 0) {
                // Clear legend ACK
                recvfrom(server_fd, (char *)ack, 1,
                         MSG_WAITALL, (struct sockaddr *) &client_address,
                         &client_address_len);
                gettimeofday(&time, 0);
                if (ack[0] < 0) {
                    cancel_timer();
                    double newRTT = calculate_time(time_map[ack[0] + 3], time);
                    if (RTTPRINT == 1) {
                        printf("newRTT: %.2f\n", newRTT);
                        printf("curRTT(old): %.2f\n", curRTT);
                    }
                    curRTT = RTT_weight * curRTT + (1 - RTT_weight) * newRTT;
                    if (RTTPRINT == 1) printf("curRTT(new): %.2f\n", curRTT);
                }
            }
        }
    }

    close(server_fd);
    printf("File successfully sent!\n");

    return 0;
}

void send_package(void) {
    int info_size; // info = header + actual info

    if (file_left > block_size) info_size = block_size + 1;
    else {
        info_size = file_left + 1;
        flag--;
        header = -1;
    }

    if (header >= 0) header = (header + 1) % 128;
    buffer[0] = header;

    int idx = header + 3;
    gettimeofday(&time_map[idx], 0);
    if (sendto(server_fd, (const char *)buffer, info_size,
               MSG_CONFIRM, (const struct sockaddr *) &client_address,
               client_address_len) == -1) printf("Fail to send information...\n");
    set_timer();
}

void resend_package(void) {
    if (flag == 0) exit(1);

    int info_size; // info = header + actual info

    if (file_left > block_size) info_size = block_size + 1;
    else {
        info_size = file_left + 1;
        flag--;
    }

    if (header >= 0) header = (header + 2) % 128;
    else header--;

    buffer[0] = header;
    int idx = header + 3;
    gettimeofday(&time_map[idx], 0);
    if (sendto(server_fd, (const char *)buffer, info_size,
               MSG_CONFIRM, (const struct sockaddr *) &client_address,
               client_address_len) == -1) printf("Fail to send information...\n");
    set_timer();
}

void set_timer(void) {
    it_val.it_value.tv_sec = 1.2 * curRTT / 1000000;
    it_val.it_value.tv_usec = ((int)(1.2 * curRTT)) % 1000000;
//    it_val.it_interval = it_val.it_value;
    it_val.it_interval.tv_sec = 0;
    it_val.it_interval.tv_usec = 0;
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

double calculate_time(struct timeval start_time, struct timeval end_time) {
    double comp_time = (end_time.tv_sec-start_time.tv_sec) * 1000000 + end_time.tv_usec-start_time.tv_usec;
    return comp_time;
}