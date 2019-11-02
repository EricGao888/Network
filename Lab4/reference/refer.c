// Problem 2 Terve
// Local: A, Target: B, Third Party: C
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
#include <time.h>

#define msg_len 50

int state;
int fd;
struct sockaddr_in local_address, remote_address, tmp_address;
int remote_address_len, tmp_address_len, local_address_len;
unsigned char reader[100], writer[100], buffer1[100], buffer2[100];
unsigned int rand_num;
int reconnect_times;
int n;

void initialize(uint16_t port_num);

void my_connect();

void reconnect();

// terve_msg_receive
void receive();

void clean(unsigned char arr[], int size);

int main(int argc, char* argv[]) {

    remote_address_len = sizeof(remote_address);
    tmp_address_len = sizeof(tmp_address);
    local_address_len = sizeof(local_address);

    state = 0;
    // state = 0: initial state
    // state = 1: connection request sent to B
    // state = 2: Response received from B

    memset(&remote_address, 0, remote_address_len);
    memset(&local_address, 0, local_address_len);
    memset(&tmp_address, 0, tmp_address_len);
    memset(&local_address, 0, local_address_len);

    // Bind address and port, establish socket connection
    initialize(htons(atoi(argv[1])));

    // bind can trigger SIGIO, so never put initialize below
    signal(SIGALRM, reconnect);
    signal(SIGIO, receive);

    if (fcntl(fd, F_SETOWN, getpid()) < 0) printf("Failed to supervise SIGIO\n");
    if (fcntl(fd, F_SETFL, O_ASYNC) < 0) printf("Failed to enable asynchronous operation on local fd\n");

    // Connect remote address
    while (state == 0) {
        printf("#ready: ");
        scanf("%s %s", buffer1, buffer2);
        // get rid of '\n'
        fgets(reader, sizeof(reader), stdin);
        clean(reader, sizeof(reader));
        my_connect();

        // if no response from B received and retransmission not all performed yet, block here
        while (state == 1);
        while (state == 2) {
            printf("your msg: ");
            clean(writer, sizeof(writer));
            fgets(writer, 50, stdin);
            writer[strlen(writer) - 1] = '\0';
            printf("YOUR MSG: %s\n", writer);
        }
    }

}

void initialize(uint16_t port_num) {

    // Create Socket
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = INADDR_ANY;
    local_address.sin_port = port_num;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Establish Half Connection
    if (bind(fd, (struct sockaddr *)&local_address, local_address_len) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
//    printf("Local IP address is: %s\n", inet_ntoa(server_address.sin_addr));
//    printf("Local port is: %d\n", (int) ntohs(server_address.sin_port));
}

void my_connect() {
    state = 1;

    remote_address.sin_family = AF_INET;
    // Convert IPv4 and IPv6 addresses from text to binary form

    if(inet_pton(AF_INET, buffer1, &remote_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return;
    }
    remote_address.sin_port = htons(atoi(buffer2));

    // Send connection request from local to remote
    srand(time(0));
    rand_num = rand();
    writer[0] = 5;
    for (int i = 3; i >= 0; i--) writer[i + 1] = (rand_num >> (i * 8)) & 255;
    alarm(5);
    sendto(fd, (const char *)writer, strlen(writer),
           0, (const struct sockaddr *) &remote_address,
           remote_address_len);
    reconnect_times = 0;
}

void reconnect() {
    if (reconnect_times == 2) {
        printf("#failure: %s %s\n", buffer1, buffer2);
        state = 0;
        return;
    }
    reconnect_times++;
    alarm(5);
    sendto(fd, (const char *)writer, strlen(writer),
           MSG_CONFIRM, (const struct sockaddr *) &remote_address,
           remote_address_len);
}

void receive() {
//    printf("Received!!!\n");
    // Receive message from remote address
    // Use non-blocking fashion to perform asynchronous operation
    clean(reader, sizeof(reader));
    n = recvfrom(fd, (char *)reader, msg_len,
                 MSG_DONTWAIT, (struct sockaddr *) &tmp_address,
                 &tmp_address_len);
    reader[n] = '\0';

//    printf("Test!");
//    printf("%u", reader[0]);
//    fflush(stdout);

    switch (state) {
        case 0:
            printf("#session request from: %s %d\n", inet_ntoa(tmp_address.sin_addr), (int) ntohs(tmp_address.sin_port));
            printf("#ready: ");
            clean(writer, sizeof(writer));
            scanf("%s", writer);

            while (writer[0] != 'y' && writer[0] != 'n') {
                printf("#ready: ");
                scanf("%s", writer);
            }

            if (writer[0] == 'y') {
                // Update State
                state = 2;
                writer[0] = 6;
            }
            else if (writer[0] == 'n') {
                writer[0] = 7;
                // state = 0;
            }

            for (int i = 1; i <= 4; i++) {
                writer[i] = reader[i];
            }

            sendto(fd, (const char *)writer, strlen(writer),
                   MSG_CONFIRM, (const struct sockaddr *) &tmp_address,
                   tmp_address_len);
            break;

        case 1:
            // Response from B
            if (tmp_address.sin_addr.s_addr == remote_address.sin_addr.s_addr && tmp_address.sin_port == remote_address.sin_port) {
                int flag = 1;
                if (reader[0] == 6) {
//                    printf("Ack Received!!");
                    fflush(stdout);
                    for (int i = 3; i >= 0; i--) {
                        // pay attention to the priority of operand
                        if (((rand_num >> 8 * i) & 255) != reader[i + 1]) {
                            printf("%u\n", (rand_num >> 8 * i) & 255);
                            printf("%u\n", reader[i+1]);
                            fflush(stdout);
                            flag = 0;
                            break;
                        }
                    }
                    // Connection established successfully
                    if (flag == 1) {
                        alarm(0);
                        // Update State
                        state = 2;
                        printf("#success: %s %s\n", buffer1, buffer2);
                    }
                }
            }
                // Response from C
            else {
                printf("#session request from: %s %d\n", inet_ntoa(tmp_address.sin_addr), (int) ntohs(tmp_address.sin_port));
            }
            break;
    }

    // Discard all packages without signals
    while (recvfrom(fd, (char *)reader, msg_len,
                    MSG_DONTWAIT, (struct sockaddr *) &tmp_address,
                    &tmp_address_len) != -1);
    clean(reader, sizeof(reader));
}

void clean(unsigned char arr[], int size) {
    memset(arr, 0, sizeof(unsigned char) * size);
}