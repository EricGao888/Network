#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
  
int main() 
{ 
    int fd; 
  
    // FIFO file path 
    char * fifo = "server_queue"; 
    char buf[100];
    while (1) 
    { 
        // Open FIFO for write only 
        fd = open(fifo, O_WRONLY); 
  
        // Take an input from user. 
        fgets(buf, 100, stdin); 
  
        // Write the input string on FIFO 
        // and close it 
        write(fd, buf, strlen(buf) + 1); 
        close(fd); 
        break;
    } 
    return 0; 
} 