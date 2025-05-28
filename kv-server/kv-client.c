// kv-client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int sockfd = -1;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void connect_to_server(char *ip, int port) {
    if (sockfd != -1) {
        printf("Error: Already connected\n");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        sockfd = -1;
        printf("Error: Connection failed\n");
        return;
    }

    printf("OK\n");
}

void disconnect_from_server() {
    if (sockfd == -1) {
        printf("Error: Not connected\n");
        return;
    }

    write(sockfd, "disconnect", 10);
    close(sockfd);
    sockfd = -1;
    printf("OK\n");
}

void send_command(const char *cmd) {
    if (sockfd == -1) {
        printf("Error: Not connected\n");
        return;
    }

    write(sockfd, cmd, strlen(cmd));

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE - 1);
    printf("%s", buffer);
}

int main(int argc, char *argv[]) {
    char input[BUFFER_SIZE];

    printf("KV Client started (type 'connect <ip> <port>' to connect):\n");
    while (1) {
        printf("> ");
        memset(input, 0, BUFFER_SIZE);
        fgets(input, BUFFER_SIZE - 1, stdin);
        input[strcspn(input, "\n")] = 0;

        if (strncmp(input, "connect", 7) == 0) {
            char ip[100];
            int port;
            if (sscanf(input + 8, "%s %d", ip, &port) == 2) {
                connect_to_server(ip, port);
            } else {
                printf("Usage: connect <ip> <port>\n");
            }
        } else if (strncmp(input, "disconnect", 10) == 0) {
            disconnect_from_server();
        } else if (strncmp(input, "create", 6) == 0 || strncmp(input, "read", 4) == 0 || 
                   strncmp(input, "update", 6) == 0 || strncmp(input, "delete", 6) == 0) {
            send_command(input);
        } else if (strcmp(input, "exit") == 0) {
            disconnect_from_server();
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    return 0;
}
