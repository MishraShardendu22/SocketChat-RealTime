#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd, newSockFd, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error on binding");

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newSockFd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
    if (newSockFd < 0)
        error("Error on accept");

    while (1) {
        bzero(buffer, 256);
        int n = read(newSockFd, buffer, 255);
        if (n < 0)
            error("Error reading from socket");

        printf("Client: %s", buffer);

        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        n = write(newSockFd, buffer, strlen(buffer));
        if (n < 0)
            error("Error writing to socket");

        if (strncmp("Bye", buffer, 3) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }

    close(newSockFd);
    close(sockfd);
    return 0;
}  