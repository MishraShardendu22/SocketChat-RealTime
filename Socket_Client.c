#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <Server IP> <Server Port> [Client Port]\n", argv[0]);
        exit(1);
    }

    int sockfd, portno, clientPort = 0;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("Error opening socket");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[2]);
    if (argc >= 4) {
        clientPort = atoi(argv[3]);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(portno);

    /* If a client port was provided and non-zero, bind the socket to that local port.
       Otherwise let the OS pick an ephemeral port (recommended). */
    if (clientPort > 0) {
        bzero((char *)&cli_addr, sizeof(cli_addr));
        cli_addr.sin_family = AF_INET;
        cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        cli_addr.sin_port = htons(clientPort);

        if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
            error("Error on binding");
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error connecting to server");

    /* After connect, determine the local port in use (ephemeral or bound) */
    socklen_t len = sizeof(cli_addr);
    if (getsockname(sockfd, (struct sockaddr *)&cli_addr, &len) < 0) {
        error("getsockname failed");
    }
    int local_port = ntohs(cli_addr.sin_port);
    printf("Connected to server %s on port %d from client port %d\n", argv[1], portno, local_port);

    char buffer[256];
    while (1) {
        bzero(buffer, 256);
        printf("You: ");
        fgets(buffer, 255, stdin);
        
        int n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("Error writing to socket");

        if (strncmp("Bye", buffer, 3) == 0) {
            printf("Client Exit...\n");
            break;
        }

        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
            error("Error reading from socket");
        
        printf("Server: %s", buffer);
    }

    close(sockfd);
    return 0;
}