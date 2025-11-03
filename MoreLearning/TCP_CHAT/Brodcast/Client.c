#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // connect to localhost

    // connect to server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect failed");
        close(sock);
        return 1;
    }

    printf("Connected to server. Type messages and press Enter.\n");

    fd_set fds;
    char buffer[256];

    while (1) {
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        FD_SET(STDIN_FILENO, &fds);

        int max_sd = sock;

        // wait for input from either keyboard or server
        if (select(max_sd + 1, &fds, NULL, NULL, NULL) < 0) {
            perror("select failed");
            break;
        }

        // if message from server
        if (FD_ISSET(sock, &fds)) {
            memset(buffer, 0, sizeof(buffer));
            int n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0) {
                printf("Disconnected from server.\n");
                break;
            }
            printf("Server: %s", buffer);
        }

        // if input from user
        if (FD_ISSET(STDIN_FILENO, &fds)) {
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            if (strcmp(buffer, "exit\n") == 0)
                break;
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
