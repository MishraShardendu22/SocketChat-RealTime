// UDP Server Code ()
// The server creates a UDP socket, binds it to a specific port to listen for incoming datagrams, and then waits to receive a message from a client.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12000
#define MAX_BUFFER_SIZE 2048

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Create UDP socket [14, 21]
    // AF_INET for IPv4, SOCK_DGRAM for UDP [14, 21]
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure [22]
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    serverAddr.sin_port = htons(PORT); // Port number, converted to network byte order [23]

    // 3. Bind the socket to the server address and port [14, 20]
    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is ready to receive on port %d\n", PORT);

    // 4. Wait to receive a message from a client [11, 14]
    // recvfrom() is a blocking call [24]
    ssize_t n = recvfrom(serverSocket, (char *)buffer, MAX_BUFFER_SIZE, 0, 
                         (struct sockaddr *) &clientAddr, &clientAddrLen);
    
    if (n < 0) {
        perror("recvfrom failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0'; // Null-terminate the received string
    
    // Print received message and client's address info
    printf("Received message from client %s:%d\n", 
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    printf("Message: %s\n", buffer);

    // 5. Close the socket [25]
    close(serverSocket);
    
    return 0;
}
