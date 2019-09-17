#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
  
int main() { 
    int fd_ser;  
    int fd_cli;
    int requestLen;
    int responseLen;
    const int unit_size = 100;

    // FIFO file path 
    char * fifo_ser = "server_queue"; 
    char * fifo_cli = "client_queue"; 
    char request[100]; // information in server queue
    // char response[1000000]; // information in client queue
    char *response = (char*)malloc(unit_size * sizeof(char));
    responseLen = unit_size;
    char buf[unit_size];
    mkfifo(fifo_cli, 0666);

    while (1) { 
        // Open FIFO for write only 
        fd_ser = open(fifo_ser, O_WRONLY); 
  
        // Take an input from user. 
        fgets(request, 100, stdin); 
  
        // Write the input string on FIFO 
        // and close it 
        write(fd_ser, request, strlen(request) + 1); 
        close(fd_ser); 

        // Open FIFO for read only 
        fd_cli = open(fifo_cli, O_RDONLY); 
  
        // Read the response from FIFO 
        // and close it 
        // allocate memory dynamically and read in while loop (read and write in pipeline is concurrent)
        // Detect the ending by the return value of read function
        int cnt = 0;
        int flag = read(fd_cli, buf, unit_size - 1);
        while (flag > 0) {
            buf[flag] = '\0';
            strcat(response, buf);
            response = (char*)realloc(response, responseLen + unit_size);
            responseLen += unit_size;
            cnt += flag;
            flag = read(fd_cli, buf, unit_size - 1);
        }
        printf("%s", response);   
        close(fd_cli); 
        break;
    } 
    return 0; 
} 