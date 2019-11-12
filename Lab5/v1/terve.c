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
#include <setjmp.h>

#define msg_len 50

int state;
int fd;
struct sockaddr_in local_address, remote_address, tmp_address;
int remote_address_len, tmp_address_len, local_address_len;
unsigned char reader[100], writer[100], buffer1[100], buffer2[100], junk_buffer[100];
unsigned int rand_num;
int reconnect_times;
int n;
jmp_buf env;
int silence;

void initialize(uint16_t port_num);

void my_connect();

void reconnect();

// terve_msg_receive
void receive();

void clean(unsigned char arr[], int size);

void terve_quit();

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
    signal(SIGQUIT, terve_quit);

    if (fcntl(fd, F_SETOWN, getpid()) < 0) printf("Failed to supervise SIGIO\n");
    if (fcntl(fd, F_SETFL, O_ASYNC) < 0) printf("Failed to enable asynchronous operation on local fd\n");

    while (1) {
        sigsetjmp(env, 1);
        switch (state) {
            case 0:
                printf("#ready: ");
                fgets(reader, 50, stdin);
                // get rid of '\n'
                reader[strlen(reader) - 1] = '\0';
                my_connect();
                break;

            case 1:
                while(state == 1);
                break;

            case 2:
                printf("\n#session request from: %s %d\n", inet_ntoa(tmp_address.sin_addr), (int) ntohs(tmp_address.sin_port));
                printf("#ready: ");
                clean(writer, sizeof(writer));
                fgets(writer, msg_len, stdin);
                writer[strlen(writer) - 1] = '\0';

                while (writer[0] != 'y' && writer[0] != 'n') {
                    printf("#ready: ");
                    fgets(writer, msg_len, stdin);
                    writer[strlen(writer) - 1] = '\0';
                }

                for (int i = 1; i <= 4; i++) {
                    writer[i] = reader[i];
                }

                if (writer[0] == 'y') {
                    // Update State
                    state = 3;
                    // Initialize Heart Rate PRO
                    alarm(5);
                    silence = 0;
                    writer[0] = 6;
                    rand_num = 0;
                    for (int i = 4; i >= 1; i--) {
                        rand_num = (reader[i] << ((i - 1) * 8)) | rand_num;
                    }
                    remote_address.sin_family = AF_INET;
                    remote_address.sin_addr.s_addr = tmp_address.sin_addr.s_addr;
                    remote_address.sin_port = tmp_address.sin_port;
                }
                else if (writer[0] == 'n') {
                    writer[0] = 7;
                    state = 0;
                }

                sendto(fd, (const char *)writer, strlen(writer),
                       MSG_CONFIRM, (const struct sockaddr *) &tmp_address,
                       tmp_address_len);

                clean(writer, sizeof(writer));
                break;

            case 3:
                printf("your msg: ");
                clean(writer, sizeof(writer));
                clean(junk_buffer, sizeof(junk_buffer));
                fgets(junk_buffer, 50, stdin);
                junk_buffer[strlen(junk_buffer) - 1] = '\0';
                writer[0] = 8;
                for (int i = 3; i >= 0; i--) writer[i + 1] = (rand_num >> (i * 8)) & 255;
                writer[5] = '\0';
                strcat(writer, junk_buffer);
                sendto(fd, (const char *)writer, strlen(writer),
                       MSG_CONFIRM, (const struct sockaddr *) &remote_address,
                       remote_address_len);
                clean(writer, sizeof(writer));
                clean(junk_buffer, sizeof(junk_buffer));
                break;
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

//    printf("%s\n", reader);
    for (int i = 0; i < strlen(reader); i++) {
        if (reader[i] == ' ') {
            strcpy(buffer2, reader + i + 1);
            reader[i] = '\0';
            break;
        }
    }
    strcpy(buffer1, reader);
//            printf("%s\n", buffer1);
//            printf("%s\n", buffer2);
    clean(reader, sizeof(reader));

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
    clean(writer, sizeof(writer));
    writer[0] = 5;
    for (int i = 3; i >= 0; i--) writer[i + 1] = (rand_num >> (i * 8)) & 255;
    alarm(5);
    sendto(fd, (const char *)writer, strlen(writer),
           0, (const struct sockaddr *) &remote_address,
           remote_address_len);
    reconnect_times = 0;
//    printf("Request Sent!\n");
}

void reconnect() {
    if (state == 1) {
        if (reconnect_times >= 2) {
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

    // Perform Heart Rate PRO
    else if (state == 3) {
        if (silence >= 3) {
            printf("#The other party is dead in accident, session terminated...\n");
            fflush(stdout);
            exit(0);
        }
        char tmp_buffer[100];
        tmp_buffer[0] = 10;
        for (int i = 3; i >= 0; i--) tmp_buffer[i + 1] = (rand_num >> (i * 8)) & 255;
        tmp_buffer[5] = '\0';
        sendto(fd, (const char *)tmp_buffer, strlen(tmp_buffer),
               MSG_CONFIRM, (const struct sockaddr *) &remote_address,
               remote_address_len);
        silence++;
        alarm(5);
    }
}

void receive() {
    if (state == 2) return;
//    printf("Received!!!\n");
    fflush(stdout);
    // Receive message from remote address
    // Use non-blocking fashion to perform asynchronous operation
    clean(reader, sizeof(reader));
    n = recvfrom(fd, (char *)reader, msg_len,
            MSG_DONTWAIT, (struct sockaddr *) &tmp_address,
                 &tmp_address_len);
    reader[n] = '\0';

    // Discard all packages without signals
    while (recvfrom(fd, (char *)junk_buffer, msg_len,
                    MSG_DONTWAIT, (struct sockaddr *) &tmp_address,
                    &tmp_address_len) != -1);
    clean(junk_buffer, sizeof(junk_buffer));

    switch (state) {
        case 0:
            if (reader[0] == 5) {
                state = 2;
                siglongjmp(env, 0);
            }
            break;

        case 1:
            // Response from B
            if (tmp_address.sin_addr.s_addr == remote_address.sin_addr.s_addr && tmp_address.sin_port == remote_address.sin_port) {
                int flag = 1;
                if (reader[0] == 6) {
                    for (int i = 3; i >= 0; i--) {
                        // Pay attention to the priority of operand
                        if (((rand_num >> 8 * i) & 255) != reader[i + 1]) {
                            flag = 0;
                            break;
                        }
                    }
                    // Connection established successfully
                    if (flag == 1) {
                        alarm(0);
                        // Update State
                        state = 3;
                        // Initialize Heart Rate PRO
                        alarm(5);
                        silence = 0;
                        printf("#success: %s %s\n", buffer1, buffer2);
                    }
                }
                else if (reader[0] == 7) {
                    alarm(0);
                    printf("#failure: %s %s\n", buffer1, buffer2);
                    state = 0;
                }

                // Avoid potential race condition problem: 愿有情人终成眷属
                else if (reader[0] == 5) {
                    alarm(0);
                    int tmp_rand = 0;
                    for (int i = 4; i >= 1; i--) {
                        tmp_rand = (reader[i] << ((i - 1) * 8)) | tmp_rand; // Bug, random num is inverted!
                    }
                    rand_num = rand_num > tmp_rand ? rand_num : tmp_rand;
                    state = 3;
                }
            }
                // Response from C
            else {
                printf("\n#session request from: %s %d\n", inet_ntoa(tmp_address.sin_addr), (int) ntohs(tmp_address.sin_port));
            }
            break;

        case 3:
            // Response from B
            if (tmp_address.sin_addr.s_addr == remote_address.sin_addr.s_addr && tmp_address.sin_port == remote_address.sin_port) {
                int flag = 1;
                if (reader[0] == 8) {
                    for (int i = 3; i >= 0; i--) {
                        // pay attention to the priority of operand
                        if (((rand_num >> 8 * i) & 255) != reader[i + 1]) {
                            flag = 0;
                            break;
                        }
                    }
                    // Connection established successfully
                    if (flag == 1) {
                        // Partner is alive, reset Heart Rate PRO
                        alarm(5);
                        silence = 0;
                        printf("\n#received msg: %s\n", reader + 5);
                        printf("your msg: ");
                        fflush(stdout);
                    }
                }
                else if (reader[0] == 9) {
                    for (int i = 3; i >= 0; i--) {
                        // pay attention to the priority of operand
                        if (((rand_num >> 8 * i) & 255) != reader[i + 1]) {
                            flag = 0;
                            break;
                        }
                    }
                    // Connection established successfully
                    if (flag == 1) {
                        printf("\n#session termination received\n");
                        fflush(stdout);
                        exit(0);
                    }
                }
                else if (reader[0] == 10) {
                    for (int i = 3; i >= 0; i--) {
                        // pay attention to the priority of operand
                        if (((rand_num >> 8 * i) & 255) != reader[i + 1]) {
                            flag = 0;
                            break;
                        }
                    }
                    // Connection established successfully
                    if (flag == 1) {
                        // Partner is alive, reset Heart Rate PRO
                        alarm(5);
                        silence = 0;
                    }
                }
            }
                // Response from C
            else {
                printf("\n#session request from: %s %d\n", inet_ntoa(tmp_address.sin_addr), (int) ntohs(tmp_address.sin_port));
                printf("your msg: ");
                fflush(stdout);
                clean(writer, sizeof(writer));
                writer[0] = 7;
                for (int i = 1; i <= 4; i++) {
                    writer[i] = reader[i];
                }
                sendto(fd, (const char *)writer, strlen(writer),
                       0, (const struct sockaddr *) &tmp_address,
                       tmp_address_len);
                clean(writer, sizeof(writer));
            }
            break;
    }

}

void clean(unsigned char arr[], int size) {
    memset(arr, 0, sizeof(unsigned char) * size);
}

void terve_quit() {
    if (state == 3) {
        clean(writer, sizeof(writer));
        writer[0] = 9;
        for (int i = 3; i >= 0; i--) writer[i + 1] = (rand_num >> (i * 8)) & 255;
        sendto(fd, (const char *)writer, strlen(writer),
               0, (const struct sockaddr *) &tmp_address,
               tmp_address_len);
        clean(writer, sizeof(writer));
        exit(0);
    }
    else exit(0);
}