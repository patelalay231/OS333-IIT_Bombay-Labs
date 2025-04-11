#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    int fd[2];

    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if(pid == 0){
        // Child process
        close(fd[1]); // Close write end of the pipe
        char buffer[100];
        read(fd[0], buffer, sizeof(buffer));
        printf("Child process received: %s", buffer);
        close(fd[0]); // Close read end after use
    }
    else{
        close(fd[0]); // Close read end of the pipe
        char message[] = "Hello from the parent process!\n";
        write(fd[1], message, sizeof(message));
        close(fd[1]); // Close write end after use
    }
}