// TCP Server Code ()
// The server waits for a client connection, accepts it, reads the incoming message, processes it (converts to uppercase), sends the reply, and then closes the dedicated connection socket. The welcoming socket remains open to accept new clients.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 12000
#define MAX_BUFFER_SIZE 1024

int main() {
    int serverSocket, connectionSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Create TCP "welcoming" socket of type SOCK_STREAM [18, 19]
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure and bind the socket [20, 21]
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr.sin_port = htons(PORT); // Port in network byte order

    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Put the server socket in listening state [3, 22]
    if (listen(serverSocket, 5) < 0) { // Allow up to 5 pending connections
        perror("listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("TCP server is ready to receive on port %d\n", PORT);

    while (1) {
        // 4. Accept a client connection (blocking call) [11, 14, 23]
        // A new socket is created for this specific client
        connectionSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (connectionSocket < 0) {
            perror("accept failed");
            continue; // Continue to the next iteration
        }
        
        printf("Connection accepted from %s:%d\n", 
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // 5. Read the message from the client [3, 24]
        ssize_t n = read(connectionSocket, buffer, MAX_BUFFER_SIZE);
        if (n < 0) {
            perror("read failed");
            close(connectionSocket);
            continue;
        }
        buffer[n] = '\0';
        printf("Received Message: %s\n", buffer);

        // 6. Process the data (convert to uppercase)
        for (int i = 0; i < n; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        // 7. Write the reply back to the client [3, 24]
        write(connectionSocket, buffer, n);
        printf("Sent Reply: %s\n", buffer);
        
        // 8. Close the connection socket for this client [25]
        close(connectionSocket);
        printf("Connection closed.\n\n");
    }
    
    close(serverSocket); // Unreachable in this loop, but good practice
    return 0;
}
