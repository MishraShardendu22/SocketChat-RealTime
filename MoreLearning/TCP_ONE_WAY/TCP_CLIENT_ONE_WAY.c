#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int port = 6000;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to server!\n");
    printf("Type your messages (type 'exit' or 'quit' to disconnect):\n");

    char buffer[1024];
    while (1) {
        printf("You: ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // Send message to server
        send(sockfd, buffer, strlen(buffer), 0);

        // Check for exit condition
        if (strncmp(buffer, "exit", 4) == 0 || strncmp(buffer, "quit", 4) == 0) {
            printf("Disconnecting...\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
