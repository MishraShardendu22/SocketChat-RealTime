#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

int main() {
    // make server
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0){
        perror("Socket creation failed");
        return 1;
    }

    int port = 5000;
    struct sockaddr_in server;
    server.sin_family = AF_INET; // IPv4
    server.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    server.sin_port = htons(port); // Port number

    // bind 
    if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    // make client 
    struct sockaddr_in client;
    socklen_t cli_len = sizeof(client);

    // read file and send to client
    char buffer[BUFFER_SIZE];
    char ack[10];
    size_t n;
    FILE *fp;

    // Wait for client request
    printf("Waiting for client request...\n");
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &cli_len);
    printf("Client request received. Starting file transfer...\n");

    fp = fopen("file.txt", "rb");
    if (!fp) {
        perror("File open failed");
        sendto(sockfd, "ERROR", 5, 0, (struct sockaddr *)&client, cli_len);
        close(sockfd);
        return 1;
    }

    int packet_count = 0;
    // Send file in chunks with acknowledgment
    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0){
        // Send packet
        sendto(sockfd, buffer, n, 0, (struct sockaddr *)&client, cli_len);
        packet_count++;
        
        // Wait for acknowledgment with timeout
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        ssize_t ack_len = recvfrom(sockfd, ack, sizeof(ack), 0, NULL, NULL);
        if (ack_len > 0) {
            printf("Packet %d sent and acknowledged (%zu bytes)\n", packet_count, n);
        } else {
            printf("No ACK received, continuing...\n");
        }
    }

    // Send end-of-file marker
    sendto(sockfd, "EOF", 3, 0, (struct sockaddr *)&client, cli_len);
    printf("File transfer complete. %d packets sent.\n", packet_count);

    fclose(fp);
    close(sockfd);
    return 0;
}
