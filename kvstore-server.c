// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/kv_store.sock"
#define MAX_KEYS 100
#define KEY_SIZE 64
#define VALUE_SIZE 256
#define BUFFER_SIZE 512

typedef struct {
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} KVPair;

KVPair store[MAX_KEYS];
int count = 0;

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        char command[10], key[KEY_SIZE], value[VALUE_SIZE];
        memset(command, 0, sizeof(command));
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        // Parse command
        sscanf(buffer, "%s %s %[^\n]", command, key, value);

        if (strcasecmp(command, "SET") == 0) {
            int updated = 0;
            for (int i = 0; i < count; i++) {
                if (strcmp(store[i].key, key) == 0) {
                    strncpy(store[i].value, value, VALUE_SIZE);
                    updated = 1;
                    break;
                }
            }
            if (!updated && count < MAX_KEYS) {
                strncpy(store[count].key, key, KEY_SIZE);
                strncpy(store[count].value, value, VALUE_SIZE);
                count++;
            }
            write(client_fd, "OK\n", 3);
        }
        else if (strcasecmp(command, "GET") == 0) {
            for (int i = 0; i < count; i++) {
                if (strcmp(store[i].key, key) == 0) {
                    write(client_fd, store[i].value, strlen(store[i].value));
                    write(client_fd, "\n", 1);
                    goto done;
                }
            }
            write(client_fd, "NOT_FOUND\n", 10);
        }
        else {
            write(client_fd, "ERROR: Unknown command\n", 23);
        }
        done:;
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;

    unlink(SOCKET_PATH);
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Key-Value Store Server started at %s\n", SOCKET_PATH);

    while (1) {
        if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
            perror("accept");
            continue;
        }
        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
