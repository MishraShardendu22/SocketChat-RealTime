#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    int port = 5000;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("UDP Server listening on port %d...\n", port);
    
    char buffer[256];
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client, &client_len);

    buffer[n] = '\0';
    printf("Received from client: %s\n", buffer);

    close(sockfd);
    return 0;
}
