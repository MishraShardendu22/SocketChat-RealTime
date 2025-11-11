#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // s
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int port = 6000;
    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    // b
    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    // l
    listen(sockfd, 1);

    
    // a
    socklen_t cli_len = sizeof(client);
    int newsockfd = accept(sockfd, (struct sockaddr *)&client, &cli_len);

    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);
        
        if (n <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        
        printf("Client: %s", buffer);
        
        // Check for exit condition
        if (strncmp(buffer, "exit", 4) == 0 || strncmp(buffer, "quit", 4) == 0) {
            printf("Client requested to quit. Closing connection.\n");
            break;
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
