#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8080);

    char buffer[256];
    socklen_t len = sizeof(server);

    while (1) {
        printf("You: ");
        fgets(buffer, sizeof(buffer), stdin);
        sendto(sockfd, buffer, strlen(buffer), 0, 
               (struct sockaddr *)&server, sizeof(server));

        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                 (struct sockaddr *)&server, &len);
        printf("Server: %s", buffer);
    }

    close(sockfd);
    return 0;
}
