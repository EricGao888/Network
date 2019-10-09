// Problem 1 Client
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/time.h>

#include <limits.h>

#define MAXBUFSZM 1000000
int main(int argc, char const *argv[]) {
    int server_fd, cnt, fd;
    struct sockaddr_in server_address;
    int server_address_len = sizeof(server_address);
    char buffer[MAXBUFSZM];
    char file_name[MAXBUFSZM];
    char user_name[LOGIN_NAME_MAX];
    char host_name[HOST_NAME_MAX];
    char flag[0];
    struct timeval start_time, end_time;
    int file_size;

    file_size = 0;
    // Add path to filename
    strcpy(file_name, "/tmp/");
    strcat(file_name, (char*)argv[1]);
    fflush(stdout);
    memset(&server_address, 0, server_address_len);

    // Get hostname and username
    if (gethostname(host_name, HOST_NAME_MAX)) {
        perror("gethostname");
        return EXIT_FAILURE;
    }
//    printf("%s\n", host_name);

    if (getlogin_r(user_name, LOGIN_NAME_MAX)) {
        perror("getlogin_r");
        return EXIT_FAILURE;
    }
//    printf("%s\n", user_name);



    // Filling server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[3]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[2], &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    if (connect(server_fd, (struct sockaddr *)&server_address, server_address_len) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send request
    write(server_fd, file_name, strlen(file_name) + 1);
    fflush(stdout);

    // Process response
    read(server_fd, flag, 1);
    if (flag[0] == '0') {
        printf("File does not exist!\n");
        exit(0);
    }
    else if (flag[0] == '1') {
        printf("File is empty!\n");
        exit(0);
    }

    else if (flag[0] == '2') {
        // Create new filename
        strcat(file_name, strtok(host_name, "."));
        strcat(file_name, user_name);

        if ((fd = creat(file_name, S_IRUSR | S_IWUSR)) < 0) {
            perror("creat() error");
            return -1;
        }
        printf("File created: %s\n", file_name);

        // Record start time
        gettimeofday(&start_time, 0);

        cnt = read(server_fd, buffer, sizeof(buffer));
        while (cnt > 0) {
            file_size += cnt;
            write(fd, buffer, cnt);
            cnt = read(server_fd, buffer, sizeof(buffer));
        }
        close(fd);

        // Record end time
        gettimeofday(&end_time, 0);
        long long comp_time = ((end_time.tv_sec-start_time.tv_sec) * 1000000LL + end_time.tv_usec-start_time.tv_usec) / 1000LL;

        // Print statics
        printf("========= Printing statics... =========\n");
        printf("Completion time: %lld milliseconds\n", comp_time);
        printf("Transmission speed: %.2f bps\n", 1.00 * file_size / (comp_time) * 1000);
        printf("Total bytes received from server: %d bytes\n", file_size);
        fflush(stdout);
    }
    else {
        printf("Unkown file access error!\n");
    }
    close(server_fd);

    return 0;
}
