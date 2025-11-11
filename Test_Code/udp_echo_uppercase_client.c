// UDP Client Code ()
// The client sends a message and then waits for the server's reply.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Use localhost (loopback address)
#define PORT 12000
#define MAX_BUFFER_SIZE 2048

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];
    const char *message = "hello, this is a udp test!";
    
    // 1. Create UDP socket [12, 19]
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure [13]
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT); // Server's port number [14]
    
    // Convert IP address from text to binary form
    if (inet_aton(SERVER_IP, &serverAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid address\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Send the message to the server [2]
    sendto(clientSocket, message, strlen(message), 0, 
           (const struct sockaddr *) &serverAddr, sizeof(serverAddr));
           
    printf("Message sent to server: %s\n", message);

    // 4. Wait to receive the reply from the server [2]
    socklen_t serverAddrLen = sizeof(serverAddr);
    ssize_t n = recvfrom(clientSocket, (char *)buffer, MAX_BUFFER_SIZE, 0,
                         (struct sockaddr *) &serverAddr, &serverAddrLen);

    if (n < 0) {
        perror("recvfrom failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0'; // Null-terminate the string
    printf("Reply from server: %s\n", buffer);

    // 5. Close the socket [18]
    close(clientSocket);
    
    return 0;
}
