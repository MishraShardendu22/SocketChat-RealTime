// UDP Client Code ()
// The client creates a UDP socket and sends a message to the server's specific IP address and port.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Use localhost for testing
#define PORT 12000
#define MAX_BUFFER_SIZE 2048

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    const char *message = "Hello from UDP client!";

    // 1. Create UDP socket [13, 21]
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure [27]
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(PORT); // Server port in network byte order [23]
    
    // Convert IP address from string to binary form
    if (inet_aton(SERVER_IP, &serverAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid address\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // 3. Send the message to the server [11, 13]
    // The OS will automatically assign an ephemeral port to the client socket
    sendto(clientSocket, message, strlen(message), 0, 
           (const struct sockaddr *) &serverAddr, sizeof(serverAddr));

    printf("Message sent to server.\n");

    // 4. Close the socket [25]
    close(clientSocket);
    
    return 0;
}
