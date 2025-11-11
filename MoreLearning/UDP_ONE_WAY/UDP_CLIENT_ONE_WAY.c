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
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    const char *msg = "Hello from UDP client";
    
    sendto(sockfd, msg, strlen(msg), 0, 
           (struct sockaddr *)&server, sizeof(server));

    printf("Message sent to server: %s\n", msg);

    close(sockfd);
    return 0;
}
