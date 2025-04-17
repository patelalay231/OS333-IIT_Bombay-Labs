#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "unix_socket"

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_un serv_addr;
    /* file path */
    char *filepath = argv[1];

    /* create socket, get sockfd handle */
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");

    /* fill in server address */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCK_PATH); 

    /* open file */
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("ERROR opening file");
        close(sockfd);
        return 1;
    }
    /* read file content */ 
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);  // Reset to beginning of file

    // Allocate memory to hold the file contents
    char *buffer = (char *)malloc(filesize + 1);  // +1 for null-terminator
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }
    fread(buffer, 1, filesize, fp);
    buffer[filesize] = '\0';

    n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (n < 0) 
        perror("ERROR writing to socket");

    close(sockfd);
    fclose(fp);
    free(buffer);
    return 0;
}