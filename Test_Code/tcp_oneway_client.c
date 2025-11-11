// TCP Client Code ()
// The client creates a socket, connects to the server, sends a message, and then closes the connection.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Use localhost for testing
#define PORT 12000

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    const char *message = "Hello from TCP client!";

    // 1. Create a TCP socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    // 2. Configure server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        perror("invalid address/ address not supported");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Connect to the server (blocking call)
    // This initiates the three-way handshake
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server.\n");

    // 4. Send the message to the server
    send(clientSocket, message, strlen(message),0);
    printf("Message sent.\n");

    // 5. Close the socket, which terminates the TCP connection
    close(clientSocket);
    
    return 0;
}
