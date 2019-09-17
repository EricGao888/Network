#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>

int requestCnt = 0;
int fd_ser;  
int fd_cli;
int requestLen;

const int unit_size = 100;
int responseLen = 100;

// FIFO file path 
char * fifo_ser = "server_queue"; 
char * fifo_cli = "client_queue"; 
char request[100]; // information in server queue
// char response[1000000]; // information in client queue
char *response;
char buf[100];

void sendRequest(void);
void retxreq(int signum);

int main() { 
    signal(SIGALRM, retxreq);
    response = (char*)malloc(unit_size * sizeof(char));
    mkfifo(fifo_cli, 0666);
     // Take an input from user. 
    fgets(request, 100, stdin);
    alarm(2);
    sendRequest();
    // Open FIFO for read only 
    fd_cli = open(fifo_cli, O_RDONLY);  // Program is blocked here
    // Read the response from FIFO 
    // and close it 
    // allocate memory dynamically and read in while loop (read and write in pipeline is concurrent)
    // Detect the ending by the return value of read function
    int cnt = 0;
    int flag = read(fd_cli, buf, unit_size - 1);
    // if (flag > 0) alarm(0);
    while (flag > 0) {
        buf[flag] = '\0';
        strcat(response, buf);
        response = (char*)realloc(response, responseLen + unit_size);
        responseLen += unit_size;
        cnt += flag;
        flag = read(fd_cli, buf, unit_size - 1);
    }
    printf("%s", response);   
    fflush(stdout);
    close(fd_cli); 
    exit(0); 

    return 0; 
} 

void sendRequest() {
    // Open FIFO for write only 
    fd_ser = open(fifo_ser, O_WRONLY); 
    // Write the input string on FIFO 
    // and close it 
    write(fd_ser, request, strlen(request) + 1); 
    close(fd_ser); 
    requestCnt++;
}

void retxreq(int signum) {
    if (requestCnt == 3) {
        printf("%s\n", "Server no response...");
        fflush(stdout);
        exit(1);     
    }
    alarm(2);
    sendRequest();
}
