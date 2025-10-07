// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/kv_store.sock"
#define BUFFER_SIZE 512

int main() {
    int client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to key-value store server.\n");
    printf("Commands: SET <key> <value>, GET <key>, or EXIT\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin))
            break;

        if (strncmp(buffer, "EXIT", 4) == 0)
            break;

        write(client_fd, buffer, strlen(buffer));

        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
        } else {
            printf("Server closed connection.\n");
            break;
        }
    }

    close(client_fd);
    return 0;
}
