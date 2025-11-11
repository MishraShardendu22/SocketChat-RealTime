#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){
    // make client
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int port = 5000;

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET; // IPv4
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server.sin_port = htons(port);

    // connect to server
    connect(sockfd, (struct sockaddr *)&server, sizeof(server));

    // store received file
    FILE *fp = fopen("received_file.txt", "wb");

    char buffer[256];
    ssize_t bytes_received;

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
        fwrite(buffer, 1, bytes_received, fp);
    }

    fclose(fp);
    close(sockfd);

    return 0;
}