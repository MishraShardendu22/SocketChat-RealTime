// TCP Server Code ()
// The server creates a listening socket, waits for a client to connect, accepts the connection, reads the message from the client, prints it, and then closes the connection.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12000
#define MAX_BUFFER_SIZE 1024

int main() {
    int serverSocket, connectionSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Create TCP "welcoming" socket
    // AF_INET for IPv4, SOCK_STREAM for TCP
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure and bind socket to it
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    serverAddr.sin_port = htons(PORT); // Port in network byte order

    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Put the server socket in listening state
    if (listen(serverSocket, 1) < 0) { // Allow 1 pending connection
        perror("listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("TCP server is ready to receive on port %d\n", PORT);

    // 4. Accept a client connection (blocking call)
    // A new socket is created for this specific client connection
    connectionSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (connectionSocket < 0) {
        perror("accept failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    
    // A client has connected!
    printf("Connection accepted from %s:%d\n", 
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // 5. Read the message from the client
    ssize_t n = recv(connectionSocket, buffer, MAX_BUFFER_SIZE,0);
    if (n < 0) {
        perror("rec failed");
    } else {
        buffer[n] = '\0'; // Null-terminate the string
        printf("Received message: %s\n", buffer);
    }
    
    // 6. Close the connection socket for this client
    close(connectionSocket);
    
    // 7. Close the welcoming socket (in this simple one-client example)
    close(serverSocket);
    
    return 0;
}
