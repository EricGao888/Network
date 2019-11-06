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

int key_idx = 0;

unsigned char myencode(unsigned char x, unsigned int pubkey);
unsigned int parseIP(char line[]);
unsigned int parseKey(char line[]);

int main(int argc, char* argv[]) {
    int server_fd, new_fd, reader;
    struct sockaddr_in server_address, client_address;
    int server_address_len = sizeof(server_address), client_address_len = sizeof(client_address);
    unsigned char buffer[100];
    unsigned int pubkey;
    int status;
    int str_len;
    int authentication;
    pid_t k;

    // Initialization, avoid undefined behavior
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Create Socket
    server_address.sin_family = AF_INET;
    server_address.sin_port = 0;

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Establish Half Connection
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Retrieve Port Number
    if (getsockname(server_fd, (struct sockaddr *)&server_address, &server_address_len) == -1) {
        perror("getsockname() failed");
        return -1;
    }

    printf("Local IP address is: %s\n", inet_ntoa(server_address.sin_addr));
    printf("Local port is: %d\n", (int) ntohs(server_address.sin_port));


    // Read to process request
    while(1) {
        key_idx = 0;
        int n;
        n = recvfrom(server_fd, (char *)buffer, sizeof(buffer),
                     MSG_WAITALL, (struct sockaddr *) &client_address,
                     &client_address_len);
        if (n > 34) {
            printf("Illegal request dropped!\n");
            continue;
        }
        buffer[n] = '\0';

        // Check ACL, retrieve public key
        authentication = 0;
        static const char filename[] = "acl.txt";
        FILE *file = fopen(filename, "r" );
        if (file != NULL) {
            char line[100];
            while (fgets(line, sizeof(line), file) != NULL) {
                line[strlen(line) - 1] = '\0'; // Get rid of '\n'
                unsigned int acl_ip = parseIP(line);
                pubkey = parseKey(line);
                if (acl_ip == client_address.sin_addr.s_addr) {
                    authentication = 1;
                    break;
                }
            }
            fclose(file);
        }
        else {
            perror(filename);
        }

        if (authentication == 0) {
            printf("Client not in ACL, request dropped!\n");
            continue;
        }

        // Decode request
        for (; key_idx < strlen(buffer); key_idx++) {
            buffer[key_idx] = myencode(buffer[key_idx], pubkey);
        }

        // Verify Certificate

        for (int i = 0; i < 4; i++) {
            if ((int)((client_address.sin_addr.s_addr >> (i * 8)) & 255) != (int)buffer[3 - i]) authentication = 0;
        }
        if (authentication == 0) {
            printf("Client failed authentication check, request dropped!\n");
            continue; // drop request which fails certificate check
        }

        // print message from client
//        printf("Message: %s\n", buffer + 4);

        // Parse arguments
        char *arguments[strlen(buffer)];
        char *token;
        token = strtok(buffer + 4, " ");
        int idx = 0;
        while (token != NULL) { 
            arguments[idx++] = token;
            token = strtok(NULL, " ");
        }
        arguments[idx] = NULL;

        // Create new child process and perform system call

    	k = fork();
        // child code
    	if (k==0) {
            // if execution failed, terminate child
            if(execvp(arguments[0], arguments) == -1) {
                exit(1);
            }	
    	}
    }
    return 0;
}

unsigned char myencode(unsigned char x, unsigned int pubkey) {
    return x ^ (pubkey >> ((key_idx++ % 4) * 8) ^ 255);
}

unsigned int parseIP(char line[]) {
    char ip[100];
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] != ' ') {
            ip[i] = line[i];
        }
        else {
            ip[i] = '\0';
            break;
        }
    }
    struct sockaddr_in tmp;
    inet_pton(AF_INET, ip, &tmp.sin_addr);
    return tmp.sin_addr.s_addr;
}

unsigned int parseKey(char line[]) {
    char key[100];
    char *ptr;
    int i = 0;
    for (; line[i] != ' '; i++);
    i++;
    int idx = 0;
    for (; i < strlen(line); i++) {
        key[idx++] = line[i];
    }
    key[idx] = '\0';
    return strtoul(key, &ptr, 10);
}