#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // create client socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 1;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) return 1;

    char buffer[256];
    printf("Connected. Wait for server to talk first.\n");

    while (1) {
        // receive first
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        printf("Server: %s", buffer);

        // send reply
        printf("You: ");
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        if (strcmp(buffer, "exit\n") == 0) break;
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
