// kv-server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAX_PAIRS 100
#define BUFFER_SIZE 1024

typedef struct {
    int key;
    char *value;
} kv_pair;

kv_pair store[MAX_PAIRS];
int kv_count = 0;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

char* handle_create(int key, const char *value) {
    for (int i = 0; i < kv_count; ++i) {
        if (store[i].key == key) return "Error: Key already exists\n";
    }
    if (kv_count >= MAX_PAIRS) return "Error: Store full\n";

    store[kv_count].key = key;
    store[kv_count].value = malloc(strlen(value) + 1);
    strcpy(store[kv_count].value, value);
    kv_count++;

    return "OK\n";
}

char* read_value(int key) {
    for (int i = 0; i < kv_count; ++i) {
        if (store[i].key == key) {
            return store[i].value;
        }
    }
    return NULL;
}

char* update_value(int key, const char *value) {
    for (int i = 0; i < kv_count; ++i) {
        if (store[i].key == key) {
            free(store[i].value);
            store[i].value = malloc(strlen(value) + 1);
            strcpy(store[i].value, value);
            return "OK\n";
        }
    }
    return "Error: Key not found\n";
}

char* delete_value(int key) {
    for (int i = 0; i < kv_count; ++i) {
        if (store[i].key == key) {
            free(store[i].value);
            store[i] = store[kv_count - 1]; // Move last element to current position
            kv_count--;
            return "OK\n";
        }
    }
    return "Error: Key not found\n";
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("KV Server listening on port %d...\n", portno);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        printf("Client connected.\n");
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
            if (n <= 0) {
                printf("Client disconnected.\n");
                close(newsockfd);
                break;
            }

            if (strncmp(buffer, "create", 6) == 0) {
                int key;
                char value[BUFFER_SIZE];
                if (sscanf(buffer + 7, "%d %[^\n]", &key, value) == 2) {
                    char *response = handle_create(key, value);
                    write(newsockfd, response, strlen(response));
                } else {
                    char *err = "Error: Invalid create command\n";
                    write(newsockfd, err, strlen(err));
                }
            } else if (strncmp(buffer, "read", 3) == 0) {
                int key;
                if (sscanf(buffer + 4, "%d", &key) == 1) {
                    char *value = read_value(key);
                    if (value) {
                        char response[BUFFER_SIZE];
                        snprintf(response, BUFFER_SIZE, "Value: %s\n", value);
                        write(newsockfd, response, strlen(response));
                    } else {
                        char *msg = "Error: Key not found\n";
                        write(newsockfd, msg, strlen(msg));
                    }
                } else {
                    char *err = "Error: Invalid find command\n";
                    write(newsockfd, err, strlen(err));
                }
            }
            else if( strncmp(buffer, "update", 6) == 0) {
                int key;
                char value[BUFFER_SIZE];
                if (sscanf(buffer + 7, "%d %[^\n]", &key, value) == 2) {
                    char *response = update_value(key, value);
                    write(newsockfd, response, strlen(response));
                } else {
                    char *err = "Error: Invalid update command\n";
                    write(newsockfd, err, strlen(err));
                }
            } else if (strncmp(buffer, "delete", 6) == 0) {
                int key;
                if (sscanf(buffer + 7, "%d", &key) == 1) {
                    char *response = delete_value(key);
                    write(newsockfd, response, strlen(response));
                } else {
                    char *err = "Error: Invalid delete command\n";
                    write(newsockfd, err, strlen(err));
                }
            }
            else if (strncmp(buffer, "disconnect", 10) == 0) {
                char *msg = "OK\n";
                write(newsockfd, msg, strlen(msg));
                printf("Client requested disconnect.\n");
                close(newsockfd);
                break;
            } else {
                char *msg = "Error: Unknown command\n";
                write(newsockfd, msg, strlen(msg));
            }
        }
    }

    close(sockfd);
    return 0;
}
