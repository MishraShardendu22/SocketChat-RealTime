#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){
    // make server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int port = 5000;
    struct sockaddr_in server, client;
    server.sin_family = AF_INET; // IPv4
    server.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    server.sin_port = htons(port); // Port number

    // bind and listen server
    bind(sockfd, (struct sockaddr*)&server, sizeof(server));
    listen(sockfd, 5);

    // make client
    socklen_t cli_len = sizeof(client);
    int client_fd = accept(sockfd, (struct sockaddr*)&client, &cli_len);

    // open file to send the file to client
    FILE* fp = fopen("file_to_send.txt", "rb");

    // reads buffer size at once sends then next 
    char buffer[256];
    size_t n;
    
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0){
        send(client_fd, buffer, n, 0);
    }

    // close all
    fclose(fp);
    close(client_fd);
    close(sockfd);

    return 0;
}