// TCP Client Code ()
// The client creates a socket, establishes a connection to the server, sends a message, waits for and reads the reply, prints it, and then closes the connection.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12000
#define MAX_BUFFER_SIZE 1024

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];
    const char *message = "hello, this is a tcp test!";

    // 1. Create a TCP socket [18, 19]
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    // 2. Configure server address structure [20, 26]
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        perror("invalid address/ address not supported");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Connect to the server [27, 28]
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server.\n");

    // 4. Send the message to the server [3, 24]
    write(clientSocket, message, strlen(message));
    printf("Message sent: %s\n", message);

    // 5. Read the reply from the server [3, 24]
    ssize_t n = read(clientSocket, buffer, MAX_BUFFER_SIZE - 1);
    if (n < 0) {
        perror("read failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("Reply from server: %s\n", buffer);

    // 6. Close the socket, terminating the connection [25]
    close(clientSocket);
    
    return 0;
}
