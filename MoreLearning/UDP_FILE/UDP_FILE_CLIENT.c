#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

int main() {
    // create a client
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server;
    socklen_t serv_len = sizeof(server);

    int port = 5000;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    // make buffer to receive file
    char buffer[BUFFER_SIZE];
    FILE *fp;

    fp = fopen("received.txt", "wb");
    if (!fp) {
        perror("File open failed");
        return 1;
    }

    // send request to server
    printf("Sending request to server...\n");
    sendto(sockfd, "START", 5, 0, (struct sockaddr *)&server, serv_len);

    // Set timeout for receiving
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ssize_t n;
    int packet_count = 0;
    while (1) {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        
        if (n < 0) {
            printf("Timeout or error. Transfer complete.\n");
            break;
        }
        
        // Check for EOF marker
        if (n == 3 && strncmp(buffer, "EOF", 3) == 0) {
            printf("End of file marker received.\n");
            break;
        }
        
        // Check for error
        if (n == 5 && strncmp(buffer, "ERROR", 5) == 0) {
            printf("Server reported error.\n");
            break;
        }
        
        // Write data to file
        fwrite(buffer, 1, n, fp);
        packet_count++;
        
        // Send acknowledgment
        sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&server, serv_len);
        printf("Packet %d received (%zd bytes)\n", packet_count, n);
    }

    printf("File transfer complete. %d packets received.\n", packet_count);

    fclose(fp);
    close(sockfd);
    return 0;
}
