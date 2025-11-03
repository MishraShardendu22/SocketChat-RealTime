#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // create client
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return 1;
    }

    int port = 5000;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    // Connect to Server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return 1;
    }

    // Chat Loop
    char buffer[256];
    while (1) {
        // initialize array with 0
        memset(buffer, 0, sizeof(buffer));

        // Receive message from server
        ssize_t n = recv(sockfd, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        printf("Server: %s\n", buffer);

        // Get input from client user
        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(sockfd, buffer, strlen(buffer), 0);
    }

    // Close Socket
    close(sockfd);
    return 0;
}
