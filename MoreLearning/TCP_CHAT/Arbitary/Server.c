#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // create server
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        return 1;
    }

    int port = 5000;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return 1;
    }

    // listen
    if (listen(server_sock, 1) < 0) {
        return 1;
    }

    // accept
    struct sockaddr_in client;
    socklen_t cli_len = sizeof(client);
    int client_sock = accept(server_sock, (struct sockaddr *)&client, &cli_len);
    if (client_sock < 0) {
        return 1;
    }

    // Chat Loop
    char buffer[256];
    while (1) {
        // initialize array with 0
        memset(buffer, 0, sizeof(buffer));

        // Receive message from client
        ssize_t n = recv(client_sock, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        printf("Client: %s\n", buffer);

        // Get input from server user
        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(client_sock, buffer, strlen(buffer), 0);
    }

    // close
    close(client_sock);
    close(server_sock);

    return 0;
}
